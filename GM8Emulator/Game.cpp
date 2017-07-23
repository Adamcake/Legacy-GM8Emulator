#include "Game.hpp"
#include "zlib\zlib.h"

#include <iostream>

#define ZLIB_BUF_START 65536

#pragma region Helper functions for parsing the filestream - no need for these to be member functions.

// Reads a dword from the given position in the byte stream
unsigned int ReadDword(const unsigned char* pStream, unsigned int* pPos) {
	unsigned int val = pStream[(*pPos)] + (pStream[(*pPos) + 1] << 8) + (pStream[(*pPos) + 2] << 16) + (pStream[(*pPos) + 3] << 24);
	(*pPos) += 4;
	return val;
}

// Reads a double from the given position in the byte stream
double ReadDouble(const unsigned char* pStream, unsigned int* pPos) {
	double val = *(double*)(pStream + (*pPos));
	(*pPos) += 4;
	return val;
}

// Reads a null-terminated string from the given position in the byte stream
// This allocates space for the string, so you should always give it to an object whose destructor will free it.
char* ReadString(const unsigned char* pStream, unsigned int* pPos) {
	unsigned int length = ReadDword(pStream, pPos);
	char* str = (char*) malloc(length + 1);
	memcpy(str, (pStream + *pPos), length);
	str[length] = '\0';
	(*pPos) += length;
	return str;
}

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
	sprintf(tmpBuffer, "_MJD%d#RWK", ReadDword(pStream, pPos));
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
	int b;
	
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
	(*pOutBuffer) = inflatedData;
	(*pOutBufferSize) = inflatedDataSize;
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
}

Game::~Game()
{
}

bool Game::Load(const char * pFilename)
{
	// Load the entirety of the file into a memory buffer

	FILE* exe = fopen(pFilename, "rb");

	if (exe == NULL) {
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
				// Error reading frontdata
				free(imageData);
				free(data);
				free(buffer);
				return false;
			}

			// Custom image data is loaded, do whatever with it but don't keep it there because it will be freed.

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

	// Extensions - there's zero documentation on how these work, they have their own encryption, and they're not in common use. So I won't support them for now.
	pos += 4;
	for (unsigned int count = ReadDword(buffer, &pos); count > 0; count--) {
		ReadDword(buffer, &pos); // Data version - 700
		pos += ReadDword(buffer, &pos); // Extension name
		pos += ReadDword(buffer, &pos); // Some kind of extension ID? eg. "GMPrint123"

		// A list of things inside the extension, seems to refer to external files.
		for (unsigned int i = ReadDword(buffer, &pos); i > 0; i--) {
			ReadDword(buffer, &pos); // Data version, 700
			pos += ReadDword(buffer, &pos); // Filename, eg "Printing.dll" - can be either .dll or .gml
			ReadDword(buffer, &pos); // Some kind of sequence id?
			pos += ReadDword(buffer, &pos); // Extern init function name, eg "__tr_init"
			pos += ReadDword(buffer, &pos); // Always blank?

			// "compiled data" according to zach

			unsigned int ii = ReadDword(buffer, &pos);
			for (; ii; ii--) {
				ReadDword(buffer, &pos);
				pos += ReadDword(buffer, &pos);
				ReadString(buffer, &pos);
				ReadDword(buffer, &pos);
				ReadDword(buffer, &pos);
				ReadDword(buffer, &pos);

				for (unsigned int j = 0; j < 17; j++) {
					ReadDword(buffer, &pos);
				}

				ReadDword(buffer, &pos);
			}

			unsigned int iii = ReadDword(buffer, &pos);
			for (; iii; iii--) {
				ReadDword(buffer, &pos);
				ReadString(buffer, &pos);
				ReadString(buffer, &pos);
			}
		}

		pos += ReadDword(buffer, &pos);
	}


	// Triggers

	Trigger* trigger;
	pos += 4;
	for (unsigned int count = ReadDword(buffer, &pos); count > 0; count--) {

		if (! InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading trigger
			free(data);
			free(buffer);
			return true;
		}

		unsigned int dataPos = 4;
		trigger = new Trigger(ReadString(data, &dataPos));
		trigger->condition = ReadString(data, &dataPos);
		trigger->checkMoment = ReadDword(data, &dataPos);
		trigger->constantName = ReadString(data, &dataPos);
		_triggers.push_back(trigger);
	}


	// Constants

	Constant* constant;
	pos += 4;
	for (unsigned int count = ReadDword(buffer, &pos); count > 0; count--) {
		constant = new Constant(ReadString(buffer, &pos));
		constant->value = ReadString(buffer, &pos);
		_constants.push_back(constant);
	}


	// Sounds

	Sound* sound;
	pos += 4;
	for (unsigned int count = ReadDword(buffer, &pos); count > 0; count--) {

		if (!InflateBlock(buffer, &pos, &data, &dataLength, &outputSize)) {
			// Error reading sound
			free(data);
			free(buffer);
			return true;
		}

		unsigned int dataPos = 0;
		if (! ReadDword(data, &dataPos)) {
			_sounds.push_back(NULL);
			continue;
		}

		sound = new Sound(ReadString(data, &dataPos));
		pos += 4;
		sound->kind = ReadDword(data, &dataPos);
		sound->fileType = ReadString(data, &dataPos);
		sound->fileName = ReadString(data, &dataPos);

		if (ReadDword(buffer, &pos)) {
			unsigned int l = ReadDword(data, &dataPos);
			sound->data = (unsigned char*) malloc(l);
			memcpy(sound->data, (data + dataPos), l);
		}
		else {
			sound->data = NULL;
			sound->dataLength = 0;
		}

		unsigned int effects = ReadDword(data, &dataPos);

		std::cout << "Sound " << sound->getName() << " =" << effects << std::endl;


		sound->volume = ReadDouble(data, &dataPos);
		sound->pan = ReadDouble(data, &dataPos);
		sound->preload = ReadDword(data, &dataPos);
		_sounds.push_back(sound);
	}

	//Cleaning up
	free(data);
	free(buffer);
	return true;
}
