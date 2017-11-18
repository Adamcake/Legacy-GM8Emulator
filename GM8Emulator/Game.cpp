#include <list>

#include "Game.hpp"
#include "CodeAction.hpp"
#include "StreamUtil.hpp"
#include "zlib\zlib.h"


#define ZLIB_BUF_START 65536

#pragma region Helper functions for parsing the filestream - no need for these to be member functions.

// YYG's implementation of Crc32
unsigned int Crc32(const void* tmpBuffer, size_t length, unsigned long* crcTable) {
	unsigned char* buffer = (unsigned char*)tmpBuffer;
	unsigned long result = 0xFFFFFFFF;

	while (length--)
		result = (result >> 8) ^ crcTable[(result & 0xFF) ^ *buffer++];

	return result;
}

// YYG's implementation of Crc32Reflect
unsigned long Crc32Reflect(unsigned long value, char c) {
	unsigned long rValue = 0;

	for (int i = 1; i < c + 1; i++) {
		if ((value & 0x01))
			rValue |= 1 << (c - i);

		value >>= 1;
	}

	return rValue;
}

// YYG's XOR mask generator for 8.1 encryption
unsigned int GetXorMask(unsigned int* seed1, unsigned int* seed2) {
	(*seed1) = (0xFFFF & (*seed1)) * 0x9069 + ((*seed1) >> 16);
	(*seed2) = (0xFFFF & (*seed2)) * 0x4650 + ((*seed2) >> 16);
	return ((*seed1) << 16) + ((*seed2) & 0xFFFF);
}

// Decrypt GM8.1 encryption
bool Decrypt81(unsigned char* pStream, unsigned int pStreamLength, unsigned int* pPos) {
	char* tmpBuffer = new char[64];
	char* buffer = new char[64];

	// Convert hash key into UTF-16
	// OLD WAY: sprintf(tmpBuffer, "_MJD%d#RWK", ReadDword(pStream, pPos));
	sprintf_s(tmpBuffer, 64, "_MJD%d#RWK", ReadDword(pStream, pPos));
	for (size_t i = 0; i < strlen(tmpBuffer); i++) {
		buffer[i * 2] = tmpBuffer[i];
		buffer[(i * 2) + 1] = 0;
	}

	// Generate crc table
	unsigned long crcTable[256];
	const unsigned long crcPolynomial = 0x04C11DB7;

	for (int i = 0; i < 256; i++) {
		crcTable[i] = Crc32Reflect(i, 8) << 24;
		for (int j = 0; j < 8; j++)
			crcTable[i] = (crcTable[i] << 1) ^ (crcTable[i] & (1 << 31) ? crcPolynomial : 0);

		crcTable[i] = Crc32Reflect(crcTable[i], 32);
	}

	// Get the two seeds used for generating xor masks
	unsigned int seed2 = Crc32(buffer, strlen(tmpBuffer) * 2, crcTable);
	unsigned int seed1 = ReadDword(pStream, pPos);

	// Skip the part that's not gm81-encrypted
	unsigned int encPos = (*pPos) + (seed2 & 0xFF) + 0xA;

	// Decrypt the rest of the stream
	while (encPos < pStreamLength) {

		// We can't decrypt the final dword if there are less than 4 bytes in it.
		//It's just garbage anyway so it doesn't matter, leave it as it is.
		if ((pStreamLength - encPos) < 4) break;

		// Otherwise we're good to go. Decrypt dword and write it back to the stream.
		unsigned int decryptedDword = ReadDword(pStream, &encPos) ^ GetXorMask(&seed1, &seed2);
		pStream[encPos - 4] = (decryptedDword & 0x000000FF);
		pStream[encPos - 3] = (decryptedDword & 0x0000FF00) >> 8;
		pStream[encPos - 2] = (decryptedDword & 0x00FF0000) >> 16;
		pStream[encPos - 1] = (decryptedDword & 0xFF000000) >> 24;
	}

	// Clean up
	delete[] tmpBuffer;
	delete[] buffer;
	return true;
}

// Decrypt the asset data paragraphs (this exists in all gm8 versions, and on top of 8.1 encryption)
bool DecryptData(unsigned char* pStream, unsigned int* pPos) {
	unsigned char swapTable[256];
	unsigned char reverseTable[256];
	unsigned int i;

	// The swap table is between two garbage tables, these dwords specify the length.
	unsigned int garbageTable1Size = 4 * ReadDword(pStream, pPos);
	unsigned int garbageTable2Size = 4 * ReadDword(pStream, pPos);

	// Get the swap table, skip garbage.
	(*pPos) += garbageTable1Size;
	memcpy(swapTable, (pStream + (*pPos)), 256);
	(*pPos) += garbageTable2Size + 256;

	// Fill the reverse table
	for (i = 0; i < 256; i++) {
		reverseTable[swapTable[i]] = i;
	}

	// Get length of encrypted area
	unsigned int len = ReadDword(pStream, pPos);

	// Decryption first pass
	for (i = (*pPos) + len; i > (*pPos) + 1; i--) {
		pStream[i - 1] = reverseTable[pStream[i - 1]] - (pStream[i - 2] + (i - ((*pPos) + 1)));
	}

	// Decryption second pass
	unsigned char a;
	unsigned int b; //?
	
	for (i = (*pPos) + len - 1; i > (*pPos); i--) {
		b = i - (int)swapTable[(i - (*pPos)) & 0xFF];
		if (b < (*pPos)) b = (*pPos);

		a = pStream[i];
		pStream[i] = pStream[b];
		pStream[b] = a;
	}

	return true;
}


// Read and inflate a data block from a byte stream
// OutBuffer must already be initialized and the size of it must be passed in OutSize. A bigger buffer will result in less iterations, thus a faster return.
// On success, the function will overwrite OutBuffer and OutBufferSize with the new buffer and max size. OutSize contains the number of bytes in the output.
bool InflateBlock(unsigned char* pStream, unsigned int* pPos, unsigned char** pOutBuffer, unsigned int* pOutBufferSize, unsigned int* pOutSize)
{
	// The first dword is the length in bytes of the compressed data following it.
	unsigned int len = ReadDword(pStream, pPos);

	// Start inflation
	z_stream strm;
	unsigned char* inflatedDataTmp;
	unsigned int inflatedDataSize = 0;
	int ret;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	if (inflateInit(&strm) != Z_OK) {
		// Error starting inflation
		return false;
	}

	// Input chunk
	strm.next_in = pStream + (*pPos);
	strm.avail_in = len;
	strm.next_out = (*pOutBuffer);
	strm.avail_out = (*pOutBufferSize);

	ret = inflate(&strm, Z_NO_FLUSH);
	if (ret == Z_STREAM_END) {
		// Success - output stream already ended, let's go home early
		inflateEnd(&strm);
		(*pOutSize) = (*pOutBufferSize) - strm.avail_out;
		(*pPos) += len;
		return true;
	}
	else if (ret != Z_OK) {
		// Error inflating
		inflateEnd(&strm);
		return false;
	}

	// Copy new data to inflatedData
	unsigned int availOut = (*pOutBufferSize) - strm.avail_out;
	unsigned char* inflatedData = (unsigned char*)malloc(availOut);
	memcpy(inflatedData, (*pOutBuffer), availOut);
	inflatedDataSize = availOut;

	// There may be more data to be output by inflate(), so we grab that until Z_STREAM_END if we don't have it already.
	while (ret != Z_STREAM_END) {
		strm.next_out = (*pOutBuffer);
		strm.avail_out = (*pOutBufferSize);
		strm.next_in = pStream + (*pPos) + (len - strm.avail_in);

		ret = inflate(&strm, Z_NO_FLUSH);
		if ((ret != Z_OK) && (ret != Z_STREAM_END)) {
			// Error inflating
			inflateEnd(&strm);
			free(inflatedData);
			return false;
		}

		// Copy new data to inflatedData
		availOut = (*pOutBufferSize) - strm.avail_out;
		inflatedDataTmp = (unsigned char*)malloc(availOut + inflatedDataSize);
		memcpy(inflatedDataTmp, inflatedData, inflatedDataSize);
		memcpy((inflatedDataTmp + inflatedDataSize), (*pOutBuffer), availOut);
		free(inflatedData);
		inflatedData = inflatedDataTmp;
		inflatedDataSize += availOut;
	}

	// Clean up and exit
	if (inflatedDataSize > (*pOutBufferSize)) {
		free(*pOutBuffer);
		(*pOutBuffer) = inflatedData;
		(*pOutBufferSize) = inflatedDataSize;
	}
	else {
		memcpy((*pOutBuffer), inflatedData, inflatedDataSize);
		free(inflatedData);
	}

	(*pOutSize) = inflatedDataSize;
	inflateEnd(&strm);
	(*pPos) += len;
	return true;
}

#pragma endregion

////////////////////////////////
// Game class implementations //
////////////////////////////////

Game::Game()
{
	info.caption = NULL;
	info.gameInfo = NULL;
	renderer = new GameRenderer();
	roomId = 0xFFFFFFFF;
	roomOrder = NULL;
}

Game::~Game()
{
	// Destroy all loaded assets, this also calls their destructors.
	_extensions.clear();
	_triggers.clear();
	_constants.clear();
	_sounds.clear();
	_sprites.clear();
	_backgrounds.clear();
	_paths.clear();
	_scripts.clear();
	_fonts.clear();
	_timelines.clear();
	_objects.clear();
	_rooms.clear();

	// Destroy game data
	free(info.caption);
	free(info.gameInfo);

	// Destroy room order
	delete[] roomOrder;

	// Destroy renderer
	delete renderer;
}

bool Game::Load(const char * pFilename)
{
	// Load the entirety of the file into a memory buffer

	FILE* exe;
	int err = fopen_s(&exe, pFilename, "rb");

	if ((exe == NULL) || (err != 0)) {
		// Error accessing file
		return false;
	}

	fseek(exe, 0, SEEK_END);
	long fsize = ftell(exe);

	fseek(exe, 0, SEEK_SET);
	unsigned char* buffer = (unsigned char*) malloc(fsize);

	if (buffer == NULL) {
		// Failed to allocate space to load the file - gm8 games can be huge so it's best to check for this.
		return false;
	}

	size_t read = fread(buffer, 1, fsize, exe);
	fclose(exe);

	// Check if this is a valid exe

	if (fsize < 0x1B) {
		// Invalid file, too small to be an exe
		free(buffer);
		return false;
	}

	if (!(buffer[0] == 'M' && buffer[1] == 'Z')) {
		// Invalid file, not an exe
		free(buffer);
		return false;
	}

	// Find game version by searching for headers

	unsigned int pos;
	int version = 0;

	// GM8.0 header
	pos = 2000000;
	if (ReadDword(buffer, &pos) == 1234321) {
		version = 800;
		pos += 8;
	}
	else {
		// GM8.1 header
		pos = 3800004;
		for (int i = 0; i < 1024; i++) {
			if ((ReadDword(buffer, &pos) & 0xFF00FF00) == 0xF7000000) {
				if ((ReadDword(buffer, &pos) & 0x00FF00FF) == 0x00140067) {

					version = 810;
					Decrypt81(buffer, fsize, &pos);

					pos += 16;
					break;
				}
				else {
					pos -= 4;
				}
			}
		}
	}

	if (!version) {
		// No game version found
		free(buffer);
		return false;
	}

	// Read all the data blocks.

	// Init variables
	unsigned int dataLength = ZLIB_BUF_START;
	unsigned char* data = (unsigned char*) malloc(dataLength);
	unsigned int outputSize;

	// Settings Data Chunk
	pos += 4;
	if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
		// Error reading settings block
		free(data);
		free(buffer);
		return false;
	}
	else {

		unsigned int settingsPos = 0;
		settings.fullscreen = ReadDword(data, &settingsPos);
		settings.interpolate = ReadDword(data, &settingsPos);
		settings.drawBorder = !ReadDword(data, &settingsPos);
		settings.displayCursor = ReadDword(data, &settingsPos);
		settings.scaling = ReadDword(data, &settingsPos);
		settings.allowWindowResize = ReadDword(data, &settingsPos);
		settings.onTop = ReadDword(data, &settingsPos);
		settings.colourOutsideRoom = ReadDword(data, &settingsPos);
		settings.setResolution = ReadDword(data, &settingsPos);
		settings.colourDepth = ReadDword(data, &settingsPos);
		settings.resolution = ReadDword(data, &settingsPos);
		settings.frequency = ReadDword(data, &settingsPos);
		settings.showButtons = !ReadDword(data, &settingsPos);
		settings.vsync = ReadDword(data, &settingsPos);
		settings.disableScreen = ReadDword(data, &settingsPos);
		settings.letF4 = ReadDword(data, &settingsPos);
		settings.letF1 = ReadDword(data, &settingsPos);
		settings.letEsc = ReadDword(data, &settingsPos);
		settings.letF5 = ReadDword(data, &settingsPos);
		settings.letF9 = ReadDword(data, &settingsPos);
		settings.treatCloseAsEsc = ReadDword(data, &settingsPos);
		settings.priority = ReadDword(data, &settingsPos);
		settings.freeze = ReadDword(data, &settingsPos);

		settings.loadingBar = ReadDword(data, &settingsPos);
		if (settings.loadingBar) {
			unsigned int loadingDataLength = ZLIB_BUF_START;
			unsigned char* loadingData = (unsigned char*)malloc(loadingDataLength);

			if (ReadDword(data, &settingsPos)) {
				// read backdata
				if (! InflateBlock(data, &settingsPos, &loadingData, &loadingDataLength, &outputSize)) {
					// Error reading backdata
					free(loadingData);
					free(data);
					free(buffer);
					return false;
				}

				// BackData is in loadingData and has length of loadingDataLength. Do whatever with it
				// But don't keep it there because it will be overwritten and then freed.
			}
			if (ReadDword(data, &settingsPos)) {
				// read frontdata
				if (! InflateBlock(data, &settingsPos, &loadingData, &loadingDataLength, &outputSize)) {
					// Error reading frontdata
					free(loadingData);
					free(data);
					free(buffer);
					return false;
				}

				// FrontData is in loadingData and has length of loadingDataLength. Do whatever with it
				// But don't keep it there because it will be freed.
			}

			free(loadingData);
		}

		settings.customLoadImage = ReadDword(data, &settingsPos);
		if (settings.customLoadImage) {
			// Read load image data
			unsigned int imageDataLength = ZLIB_BUF_START;
			unsigned char* imageData = (unsigned char*)malloc(imageDataLength);

			if (!InflateBlock(data, &settingsPos, &imageData, &imageDataLength, &outputSize)) {
				// Error reading custom load image
				free(imageData);
				free(data);
				free(buffer);
				return false;
			}

			// Custom image data is loaded in the format of a BMP file (Always BMP I assume? Check?) Do whatever with it but don't keep it there because it will be freed.

			free(imageData);
		}

		settings.transparent = ReadDword(data, &settingsPos);
		settings.translucency = ReadDword(data, &settingsPos);
		settings.scaleProgressBar = ReadDword(data, &settingsPos);
		settings.errorDisplay = ReadDword(data, &settingsPos);
		settings.errorLog = ReadDword(data, &settingsPos);
		settings.errorAbort = ReadDword(data, &settingsPos);

		unsigned int uninit = ReadDword(data, &settingsPos);
		if (version == 810) {
			settings.treatAsZero = uninit & 1;
			settings.errorOnUninitialization = uninit & 2;
		}
		else {
			settings.treatAsZero = uninit;
			settings.errorOnUninitialization = true;
		}
	}

	// Skip over the D3D wrapper
	pos += ReadDword(buffer, &pos);
	pos += ReadDword(buffer, &pos);

	// There's yet another encryption layer on the rest of the data paragraphs.
	if (!DecryptData(buffer, &pos)) {
		// Error decrypting
		free(data);
		free(buffer);
		return false;
	}

	// Garbage fields
	pos += (ReadDword(buffer, &pos) + 6) * 4;

	// Extensions

	pos += 4;
	unsigned char* charTable = NULL;
	unsigned int count = ReadDword(buffer, &pos);
	if(count) charTable = (unsigned char*)malloc(0x200);
	_extensions.reserve(count);
	for (; count > 0; count--) {
		_extensions.push_back(Extension());
		Extension* extension = _extensions._Mylast() - 1;

		pos += 4; // Data version, 700
		extension->name = ReadString(buffer, &pos);
		extension->folderName = ReadString(buffer, &pos);

		// The list of files inside the extension
		extension->fileCount = ReadDword(buffer, &pos);
		extension->files = new ExtensionFile[extension->fileCount];
		for (unsigned int i = 0; i < extension->fileCount; i++) {
			ExtensionFile* extfile = extension->files + i;

			pos += 4; // Data version, 700
			extfile->filename = ReadString(buffer, &pos);
			extfile->kind = ReadDword(buffer, &pos);
			extfile->initializer = ReadString(buffer, &pos);
			extfile->finalizer = ReadString(buffer, &pos);

			// Functions
			extfile->functionCount = ReadDword(buffer, &pos);
			extfile->functions = new ExtensionFileFunction[extfile->functionCount];
			for (unsigned int ii = 0; ii < extfile->functionCount; ii++) {
				pos += 4; // Data version 700
				extfile->functions[ii].name = ReadString(buffer, &pos);
				extfile->functions[ii].externalName = ReadString(buffer, &pos);
				extfile->functions[ii].convention = ReadDword(buffer, &pos);
				pos += 4; // always 0?
				extfile->functions[ii].argCount = ReadDword(buffer, &pos);

				for (unsigned int j = 0; j < 17; j++) {
					extfile->functions[ii].argTypes[j] = ReadDword(buffer, &pos); // arg type - 1 for string, 2 for real
				}

				extfile->functions[ii].returnType = ReadDword(buffer, &pos); // function return type - 1 for string, 2 for real
			}

			// Constants
			extfile->constCount = ReadDword(buffer, &pos);
			extfile->consts = new ExtensionFileConst[extfile->constCount];
			for (unsigned int ii = 0; ii < extfile->constCount; ii++) {
				pos += 4; // Data version 700
				extfile->consts[ii].name = ReadString(buffer, &pos);
				extfile->consts[ii].value = ReadString(buffer, &pos);
			}
		}

		// Actual file data, including decryption
		unsigned int endpos = ReadDword(buffer, &pos);
		unsigned int dataPos = pos;
		pos += endpos;

		// File decryption - generate byte table
		int seed1 = ReadDword(buffer, &dataPos);
		int seed2 = (seed1 % 0xFA) + 6;
		seed1 /= 0xFA;
		if (seed1 < 0) seed1 += 100;
		if (seed2 < 0) seed2 += 100;

		for (unsigned int i = 0; i < 0x200; i++) {
			charTable[i] = i;
		}

		// File decryption - byte table first pass
		for (unsigned int i = 1; i < 0x2711; i++) {
			unsigned int AX = (((i * seed2) + seed1) % 0xFE) + 1;
			unsigned char b1 = charTable[AX];
			unsigned char b2 = charTable[AX + 1];
			charTable[AX] = b2;
			charTable[AX + 1] = b1;
		}

		// File decryption - byte table second pass
		for (unsigned int i = 0; i < 0x100; i++) {
			unsigned char DX = charTable[i + 1];
			charTable[DX + 0x100] = i + 1;
		}

		// File decryption - decrypting data block
		for (unsigned int i = dataPos + 1; i < pos; i++) {
			buffer[i] = charTable[buffer[i] + 0x100];
		}

		// Read the files
		for (unsigned int i = 0; i < extension->fileCount; i++) {
			if (!InflateBlock(buffer, &dataPos, &data, &dataLength, &outputSize)) {
				// Error reading file
				free(data);
				free(buffer);
				free(charTable);
				return true;
			}

			extension->files[i].dataLength = outputSize;
			extension->files[i].data = (unsigned char*)malloc(outputSize);
			memcpy(extension->files[i].data, data, outputSize);
		}

	}
	free(charTable);


	// The EXE data can contain non-existent (deleted) assets. These count as indexed assets, so we need to push these into the list, but also indicate that they don't really exist.
	// We'll handle this by creating dummy objects here for later pushing.
	Trigger dummyTrigger;
	dummyTrigger.exists = false;
	Constant dummyConstant;
	dummyConstant.exists = false;
	Sound dummySound;
	dummySound.exists = false;
	Sprite dummySprite;
	dummySprite.exists = false;
	Background dummyBackground;
	dummyBackground.exists = false;
	Path dummyPath;
	dummyPath.exists = false;
	Script dummyScript;
	dummyScript.exists = false;
	Font dummyFont;
	dummyFont.exists = false;
	Timeline dummyTimeline;
	dummyTimeline.exists = false;
	Object dummyObject;
	dummyObject.exists = false;
	Room dummyRoom;
	dummyRoom.exists = false;
	IncludeFile dummyIncludeFile;
	dummyIncludeFile.exists = false;


	// Triggers

	pos += 4;
	count = ReadDword(buffer, &pos);
	_triggers.reserve(count);
	for (; count > 0; count--) {

		if (! InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading trigger
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_triggers.push_back(dummyTrigger);
			continue;
		}

		_triggers.push_back(Trigger());
		Trigger* trigger = _triggers._Mylast() - 1;
		dataPos += 4;
		trigger->name = ReadString(data, &dataPos);
		trigger->condition = ReadString(data, &dataPos);
		trigger->checkMoment = ReadDword(data, &dataPos);
		trigger->constantName = ReadString(data, &dataPos);
	}


	// Constants

	pos += 4;
	count = ReadDword(buffer, &pos);
	_constants.reserve(count);
	for (; count > 0; count--) {
		_constants.push_back(Constant());
		Constant* constant = _constants._Mylast() - 1;
		constant->name = ReadString(buffer, &pos);
		constant->value = ReadString(buffer, &pos);
	}


	// Sounds

	pos += 4;
	count = ReadDword(buffer, &pos);
	_sounds.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading sound
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (! ReadDword(data, &dataPos)) {
			_sounds.push_back(dummySound);
			continue;
		}

		_sounds.push_back(Sound());
		Sound* sound = _sounds._Mylast() - 1;
		sound->name = ReadString(data, &dataPos);
		dataPos += 4;
		sound->kind = ReadDword(data, &dataPos);
		sound->fileType = ReadString(data, &dataPos);
		sound->fileName = ReadString(data, &dataPos);

		if (ReadDword(data, &dataPos)) {
			unsigned int l = ReadDword(data, &dataPos);
			sound->data = (unsigned char*) malloc(l);
			memcpy(sound->data, (data + dataPos), l);
		}
		else {
			sound->data = NULL;
			sound->dataLength = 0;
		}

		dataPos += 4; // Not sure what this is, appears to be unused

		sound->volume = ReadDouble(data, &dataPos);
		sound->pan = ReadDouble(data, &dataPos);
		sound->preload = ReadDword(data, &dataPos);
	}


	// Sprites

	pos += 4;
	count = ReadDword(buffer, &pos);
	_sprites.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading sprite
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_sprites.push_back(dummySprite);
			continue;
		}

		_sprites.push_back(Sprite());
		Sprite* sprite = _sprites._Mylast() - 1;
		sprite->name = ReadString(data, &dataPos);
		dataPos += 4;

		sprite->originX = ReadDword(data, &dataPos);
		sprite->originY = ReadDword(data, &dataPos);

		sprite->frameCount = ReadDword(data, &dataPos);
		if (sprite->frameCount) {
			sprite->frames = (RImageIndex*) malloc(sizeof(RImageIndex*) * sprite->frameCount);

			// Frame data
			unsigned int i;
			for (i = 0; i < sprite->frameCount; i++) {
				dataPos += 4;

				unsigned int w = ReadDword(data, &dataPos);
				unsigned int h = ReadDword(data, &dataPos);
				unsigned int pixelDataLength = ReadDword(data, &dataPos);

				if (pixelDataLength != (w * h * 4)) {
					// This should never happen
					free(data);
					free(buffer);
					return false;
				}

				// Convert BGRA to RGBA
				unsigned char* pixelData = (data + dataPos);
				unsigned int pixelDataEnd = (dataPos + pixelDataLength);
				unsigned char tmp;
				for (; dataPos < pixelDataEnd; dataPos += 4) {
					tmp = data[dataPos];
					data[dataPos] = data[dataPos + 2];
					data[dataPos + 2] = tmp;
				}

				sprite->frames[i] = renderer->MakeImage(w, h, sprite->originX, sprite->originY, pixelData);
			}

			// Collision data
			sprite->separateCollision = ReadDword(data, &dataPos);
			if (sprite->separateCollision) {
				// Separate maps
				sprite->collisionMaps = new CollisionMap[sprite->frameCount];
				for (i = 0; i < sprite->frameCount; i++) {
					dataPos += 4;

					CollisionMap* map = &(sprite->collisionMaps[i]);
					map->width = ReadDword(data, &dataPos);
					map->height = ReadDword(data, &dataPos);
					map->left = ReadDword(data, &dataPos);
					map->right = ReadDword(data, &dataPos);
					map->bottom = ReadDword(data, &dataPos);
					map->top = ReadDword(data, &dataPos);

					unsigned int maskSize = map->width * map->height;
					map->collision = new bool[maskSize];
					for (unsigned int ii = 0; ii < maskSize; ii++) {
						map->collision[ii] = ReadDword(data, &dataPos);
					}
				}
			}
			else {
				// One map

				dataPos += 4;

				CollisionMap* map = new CollisionMap();
				sprite->collisionMaps = map;
				map->width = ReadDword(data, &dataPos);
				map->height = ReadDword(data, &dataPos);
				map->left = ReadDword(data, &dataPos);
				map->right = ReadDword(data, &dataPos);
				map->bottom = ReadDword(data, &dataPos);
				map->top = ReadDword(data, &dataPos);

				unsigned int maskSize = map->width * map->height;
				map->collision = new bool[maskSize];
				for (unsigned int ii = 0; ii < maskSize; ii++) {
					map->collision[ii] = ReadDword(data, &dataPos);
				}
			}
		}
	}


	// Backgrounds

	pos += 4;
	count = ReadDword(buffer, &pos);
	_backgrounds.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading background
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_backgrounds.push_back(dummyBackground);
			continue;
		}

		_backgrounds.push_back(Background());
		Background* background = _backgrounds._Mylast() - 1;
		background->name = ReadString(data, &dataPos);
		dataPos += 8;
		background->width = ReadDword(data, &dataPos);
		background->height = ReadDword(data, &dataPos);

		if (background->width > 0 && background->height > 0) {
			unsigned int len = ReadDword(data, &dataPos);
			background->data = (unsigned char*)malloc(len);
			memcpy(background->data, (data + dataPos), len);
		}
	}


	// Paths

	pos += 4;
	count = ReadDword(buffer, &pos);
	_paths.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading path
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_paths.push_back(dummyPath);
			continue;
		}

		_paths.push_back(Path());
		Path* path = _paths._Mylast() - 1;
		path->name = ReadString(data, &dataPos);

		dataPos += 4;
		path->kind = ReadDword(data, &dataPos);
		path->closed = ReadDword(data, &dataPos);
		path->precision = ReadDword(data, &dataPos);

		path->pointCount = ReadDword(data, &dataPos);
		path->points = new PathPoint[path->pointCount];
		for (unsigned int i = 0; i < path->pointCount; i++) {
			PathPoint* p = path->points + i;
			p->x = ReadDouble(data, &dataPos);
			p->y = ReadDouble(data, &dataPos);
			p->speed = ReadDouble(data, &dataPos);
		}
	}


	// Scripts

	pos += 4;
	count = ReadDword(buffer, &pos);
	_scripts.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading script
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_scripts.push_back(dummyScript);
			continue;
		}

		_scripts.push_back(Script());
		Script* script = _scripts._Mylast() - 1;
		script->name = ReadString(data, &dataPos);

		dataPos += 4;
		script->code = ReadString(data, &dataPos);
	}


	// Fonts

	pos += 4;
	count = ReadDword(buffer, &pos);
	_fonts.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading font
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_fonts.push_back(dummyFont);
			continue;
		}

		_fonts.push_back(Font());
		Font* font = _fonts._Mylast() - 1;
		font->name = ReadString(data, &dataPos);

		dataPos += 4;
		font->fontName = ReadString(data, &dataPos);
		font->size = ReadDword(data, &dataPos);
		font->bold = ReadDword(data, &dataPos);
		font->italic = ReadDword(data, &dataPos);
		font->rangeBegin = ReadDword(data, &dataPos);
		font->rangeEnd = ReadDword(data, &dataPos);

		// Coordinate data for characters 0-255 in the bitmap.
		unsigned int dmap[0x600];
		for (unsigned int i = 0; i < 0x600; i++) {
			dmap[i] = ReadDword(data, &dataPos); // Have to do it this way instead of a memcpy so it stays endian-safe.
		}

		unsigned int w = ReadDword(data, &dataPos);
		unsigned int h = ReadDword(data, &dataPos);
		unsigned int dlen = ReadDword(data, &dataPos);

		// tbd - there are %dlen bytes here containing the font bitmap.
	}


	// Timelines

	pos += 4;
	count = ReadDword(buffer, &pos);
	_timelines.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading timeline
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_timelines.push_back(dummyTimeline);
			continue;
		}

		_timelines.push_back(Timeline());
		Timeline* timeline = _timelines._Mylast() - 1;
		timeline->name = ReadString(data, &dataPos);

		dataPos += 4;
		timeline->momentCount = ReadDword(data, &dataPos);
		timeline->moments = new IndexedEvent[timeline->momentCount];

		for (unsigned int i = 0; i < timeline->momentCount; i++) {
			timeline->moments[i].index = ReadDword(data, &dataPos);
			dataPos += 4;

			timeline->moments[i].actionCount = ReadDword(data, &dataPos);
			timeline->moments[i].actions = new CodeAction[timeline->moments[i].actionCount];

			for (unsigned int j = 0; j < timeline->moments[i].actionCount; j++) {
				if (!timeline->moments[i].actions[j].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}
		}
	}


	// Objects

	pos += 4;
	count = ReadDword(buffer, &pos);
	_objects.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading object
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_objects.push_back(dummyObject);
			continue;
		}

		_objects.push_back(Object());
		Object* object = _objects._Mylast() - 1;
		object->name = ReadString(data, &dataPos);
		dataPos += 4;

		object->spriteIndex = (int)ReadDword(data, &dataPos);
		object->solid = ReadDword(data, &dataPos);
		object->visible = ReadDword(data, &dataPos);
		object->depth = (int)ReadDword(data, &dataPos);
		object->persistent = ReadDword(data, &dataPos);
		object->parentIndex = (int)ReadDword(data, &dataPos);
		object->maskIndex = (int)ReadDword(data, &dataPos);

		dataPos += 4; // This skips a counter for the number of event lists. Should always be 11.

		IndexedEvent e;

		// Create event
		if ((int)(ReadDword(data, &dataPos)) != -1) {
			dataPos += 4;
			object->evCreateActionCount = ReadDword(data, &dataPos);
			object->evCreate = new CodeAction[object->evCreateActionCount];
			for (unsigned int i = 0; i < object->evCreateActionCount; i++) {
				if (!object->evCreate[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}
			dataPos += 4; // Should always be -1, otherwise this object has more than one create event.
		}
		
		// Destroy event
		if ((int)(ReadDword(data, &dataPos)) != -1) {
			dataPos += 4;
			object->evDestroyActionCount = ReadDword(data, &dataPos);
			object->evDestroy = new CodeAction[object->evDestroyActionCount];
			for (unsigned int i = 0; i < object->evDestroyActionCount; i++) {
				if (!object->evDestroy[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}
			dataPos += 4; // Should always be -1, otherwise this object has more than one destroy event.
		}

		// Alarm events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evAlarm.push_back(e);
		}

		// Step events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			else if (index == 0) {
				// Step
				if (object->evStep != NULL) {
					// Two step events for this object?
					free(data);
					free(buffer);
					return false;
				}

				dataPos += 4;
				object->evStepActionCount = ReadDword(data, &dataPos);
				object->evStep = new CodeAction[object->evStepActionCount];
				for (unsigned int i = 0; i < object->evStepActionCount; i++) {
					object->evStep[i].read(data, &dataPos);
				}
			}
			else if (index == 1) {
				// Begin Step
				if (object->evStepBegin != NULL) {
					// Two begin step events for this object?
					free(data);
					free(buffer);
					return false;
				}

				dataPos += 4;
				object->evStepBeginActionCount = ReadDword(data, &dataPos);
				object->evStepBegin = new CodeAction[object->evStepBeginActionCount];
				for (unsigned int i = 0; i < object->evStepBeginActionCount; i++) {
					object->evStepBegin[i].read(data, &dataPos);
				}
			}
			else if (index == 2) {
				// End Step
				if (object->evStepEnd != NULL) {
					// Two begin step events for this object?
					free(data);
					free(buffer);
					return false;
				}

				dataPos += 4;
				object->evStepEndActionCount = ReadDword(data, &dataPos);
				object->evStepEnd = new CodeAction[object->evStepEndActionCount];
				for (unsigned int i = 0; i < object->evStepEndActionCount; i++) {
					object->evStepEnd[i].read(data, &dataPos);
				}
			}
		}

		// Collision events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evCollision.push_back(e);
		}

		// Keyboard events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evKeyboard.push_back(e);
		}

		// Mouse events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evMouse.push_back(e);
		}

		// Other events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evOther.push_back(e);
		}

		// Draw event
		if ((int)(ReadDword(data, &dataPos)) != -1) {
			dataPos += 4;
			object->evDrawActionCount = ReadDword(data, &dataPos);
			object->evDraw = new CodeAction[object->evDrawActionCount];
			for (unsigned int i = 0; i < object->evDrawActionCount; i++) {
				if (!object->evDraw[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}
			dataPos += 4; // Should always be -1, otherwise this object has more than one draw event.
		}

		// Key press events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evKeyPress.push_back(e);
		}

		// Key release events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evKeyRelease.push_back(e);
		}

		// Trigger events
		while (true) {
			unsigned int index = ReadDword(data, &dataPos);
			if (index == -1) break;
			e.index = index;

			dataPos += 4;
			e.actionCount = ReadDword(data, &dataPos);
			e.actions = new CodeAction[e.actionCount];
			for (unsigned int i = 0; i < e.actionCount; i++) {

				if (!e.actions[i].read(data, &dataPos)) {
					// Error reading action
					free(data);
					free(buffer);
					return false;
				}
			}

			object->evTrigger.push_back(e);
		}

		e.actions = NULL; // Prevents important memory getting destroyed along with this stack memory
	}


	// Rooms

	pos += 4;
	count = ReadDword(buffer, &pos);
	_rooms.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading room
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_rooms.push_back(dummyRoom);
			continue;
		}

		_rooms.push_back(Room());
		Room* room = _rooms._Mylast() - 1;
		room->name = ReadString(data, &dataPos);
		dataPos += 4;

		room->caption = ReadString(data, &dataPos);
		room->width = ReadDword(data, &dataPos);
		room->height = ReadDword(data, &dataPos);
		room->speed = ReadDword(data, &dataPos);
		room->persistent = ReadDword(data, &dataPos);
		room->backgroundColour = ReadDword(data, &dataPos);
		room->drawBackgroundColour = ReadDword(data, &dataPos);
		room->creationCode = ReadString(data, &dataPos);

		// Room backgrounds
		room->backgroundCount = ReadDword(data, &dataPos);
		room->backgrounds = new RoomBackground[room->backgroundCount];
		for (unsigned int i = 0; i < room->backgroundCount; i++) {
			RoomBackground* bg = room->backgrounds + i;
			bg->visible = ReadDword(data, &dataPos);
			bg->foreground = ReadDword(data, &dataPos);
			bg->backgroundIndex = ReadDword(data, &dataPos);
			bg->x = ReadDword(data, &dataPos);
			bg->y = ReadDword(data, &dataPos);
			bg->tileHor = ReadDword(data, &dataPos);
			bg->tileVert = ReadDword(data, &dataPos);
			bg->hSpeed = ReadDword(data, &dataPos);
			bg->vSpeed = ReadDword(data, &dataPos);
			bg->stretch = ReadDword(data, &dataPos);
		}

		// Room views
		room->enableViews = ReadDword(data, &dataPos);
		room->viewCount = ReadDword(data, &dataPos);
		room->views = new RoomView[room->viewCount];
		for (unsigned int i = 0; i < room->viewCount; i++) {
			RoomView* view = room->views + i;
			view->visible = ReadDword(data, &dataPos);
			view->viewX = (int)ReadDword(data, &dataPos);
			view->viewY = (int)ReadDword(data, &dataPos);
			view->viewW = ReadDword(data, &dataPos);
			view->viewH = ReadDword(data, &dataPos);
			view->portX = ReadDword(data, &dataPos);
			view->portY = ReadDword(data, &dataPos);
			view->portW = ReadDword(data, &dataPos);
			view->portH = ReadDword(data, &dataPos);
			view->Hbor = ReadDword(data, &dataPos);
			view->Vbor = ReadDword(data, &dataPos);
			view->Hsp = ReadDword(data, &dataPos);
			view->Vsp = ReadDword(data, &dataPos);
			view->follow = ReadDword(data, &dataPos);
		}

		// Room instances
		room->instanceCount = ReadDword(data, &dataPos);
		room->instances = new RoomInstance[room->instanceCount];
		for (unsigned int i = 0; i < room->instanceCount; i++) {
			RoomInstance* instance = room->instances + i;
			instance->x = (int)ReadDword(data, &dataPos);
			instance->y = (int)ReadDword(data, &dataPos);
			instance->objectIndex = ReadDword(data, &dataPos);
			instance->id = ReadDword(data, &dataPos);
			instance->creationCode = ReadString(data, &dataPos);
		}

		// Room tiles
		room->tileCount = ReadDword(data, &dataPos);
		room->tiles = new RoomTile[room->tileCount];
		for (unsigned int i = 0; i < room->tileCount; i++) {
			RoomTile* tile = room->tiles + i;
			tile->x = (int)ReadDword(data, &dataPos);
			tile->y = (int)ReadDword(data, &dataPos);
			tile->backgroundIndex = ReadDword(data, &dataPos);
			tile->tileX = ReadDword(data, &dataPos);
			tile->tileY = ReadDword(data, &dataPos);
			tile->width = ReadDword(data, &dataPos);
			tile->height = ReadDword(data, &dataPos);
			tile->depth = (int)ReadDword(data, &dataPos);
			tile->id = ReadDword(data, &dataPos);
		}
	}


	// ... not sure
	pos += 8;

	
	// Include files

	pos += 4;
	count = ReadDword(buffer, &pos);
	_includeFiles.reserve(count);
	for (; count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading whatever this is
			free(data);
			free(buffer);
			return false;
		}

		unsigned int dataPos = 0;
		if (!ReadDword(data, &dataPos)) {
			_includeFiles.push_back(dummyIncludeFile);
			continue;
		}

		_includeFiles.push_back(IncludeFile());
		IncludeFile* file = _includeFiles._Mylast() - 1;
		dataPos += 4;

		file->filename = ReadString(data, &dataPos);
		file->filepath = ReadString(data, &dataPos);
		bool inExe = ReadDword(data, &dataPos);
		file->originalSize = ReadDword(data, &dataPos);
		inExe = inExe && ReadDword(data, &dataPos);

		if (inExe) {
			file->dataLength = ReadDword(data, &dataPos);
			file->data = new unsigned char[file->dataLength];
			memcpy(file->data, (data + dataPos), file->dataLength);
		}

		file->exportFlags = ReadDword(data, &dataPos);
		file->exportFolder = ReadString(data, &dataPos);
		file->overwrite = ReadDword(data, &dataPos);
		file->freeMemory = ReadDword(data, &dataPos);
		file->removeAtGameEnd = ReadDword(data, &dataPos);
	}


	// Game information data (the thing that comes up when you press F1)
	pos += 4;
	if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
		// Error reading game information
		free(data);
		free(buffer);
		return false;
	}

	unsigned int dataPos = 0;
	info.backgroundColour = ReadDword(data, &dataPos);
	info.separateWindow = ReadDword(data, &dataPos);
	info.caption = ReadString(data, &dataPos);
	info.left = ReadDword(data, &dataPos);
	info.top = ReadDword(data, &dataPos);
	info.width = ReadDword(data, &dataPos);
	info.height = ReadDword(data, &dataPos);
	info.showBorder = ReadDword(data, &dataPos);
	info.allowWindowResize = ReadDword(data, &dataPos);
	info.onTop = ReadDword(data, &dataPos);
	info.freezeGame = ReadDword(data, &dataPos);
	info.gameInfo = ReadString(data, &dataPos);


	// Garbage?
	pos += 4;
	count = ReadDword(buffer, &pos);
	for (; count > 0; count--) {
		pos += ReadDword(buffer, &pos);
	}


	// Room order
	pos += 4;
	count = ReadDword(buffer, &pos);
	roomOrder = new unsigned int[count];
	for (unsigned int i = 0; i < count; i++) {
		roomOrder[i] = ReadDword(buffer, &pos);
	}

	// Cleaning up
	free(data);
	free(buffer);

	return true;
}

bool Game::StartGame() {
	// Clear out the instances if there were any
	// TODO - once there's an instance list, call clear() on it here

	// Reset the room to its default value so that LoadRoom() won't ever fail when restarting
	roomId = 0xFFFFFFFF;

	// Start up game window (this will safely destroy the old one if one existed)
	if (!renderer->MakeGameWindow(&settings, _rooms[roomOrder[0]].width, _rooms[roomOrder[0]].height)) {
		// Failed to create GLFW window
		return false;
	}

	// Load first room
	LoadRoom(roomOrder[0]);

	return true;
}

bool Game::LoadRoom(unsigned int id) {
	// Exit if we're already in this room
	if (id == roomId) return true;

	// Check this id is a valid room
	if (id >= _rooms.size()) return false;

	// Get the Room object for the room we're going to
	Room* room = &_rooms[id];

	// Check room exists
	if (!room->exists) return false;

	// Update renderer
	renderer->ResizeGameWindow(room->width, room->height);
	renderer->SetGameWindowTitle(room->caption);

	// TODO: delete all non-persistent instances
	// TODO: create all instances in new room

	return true;
}



bool Game::Frame() {
	//* test code
	double xpos, ypos;
	renderer->GetCursorPos(&xpos, &ypos);
	renderer->DrawImage(31, 232, 232, 3, 4, 0, 0xFFFFFF, 0.5);
	renderer->DrawImage(31, 248, 248, 1, 1, 0, 0xFFFFFF, 1);
	renderer->DrawImage(31, 264, 344, 1, 2, 0, 0xFFFFFF, 0.5);
	renderer->DrawImage(21, xpos, ypos, 2, 2, 0, 0xFFFFFF, 1);
	

	renderer->RenderFrame();
	return !renderer->ShouldClose();
}