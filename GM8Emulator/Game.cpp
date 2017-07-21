#include "Game.hpp"
#include <zconf.h>
#include <zlib.h>

#define ZLIB_CHUNK 65536

#pragma region Helper functions for parsing the filestream - no need for these to be member functions.

// Reads a dword from the given position in the byte stream
unsigned int ReadDword(const unsigned char* pStream, unsigned int* pPos) {
	unsigned int position = *pPos;
	unsigned int val = pStream[position] + (pStream[position + 1] << 8) + (pStream[position + 2] << 16) + (pStream[position + 3] << 24);
	(*pPos) += 4;
	return val;
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
	unsigned int encPos = (*pPos) + (seed2 & 0xFF) + 6;

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

// Read and inflate a data block from the data stream
bool InflateBlock(const unsigned char* pStream, unsigned int* pPos, unsigned char* pInBuffer, unsigned char* pOutBuffer, unsigned char** pOut, unsigned int* pOutSize)
{
	// The first dword is always 0x320. Maybe we could actually check this value if we want strict integrity checking but it isn't needed.
	(*pPos) += 4;

	// Next dword is the length in bytes of the compressed data following it.
	unsigned int len = ReadDword(pStream, pPos);
	unsigned char* data = (unsigned char*) malloc(len);

	// Read bytes
	memcpy(data, (pStream + *pPos), len);
	(*pPos) += len;

	// Inflate data
	z_stream strm;
	unsigned char* inflatedData = nullptr;
	unsigned char* inflatedDataTmp;
	unsigned int inflatedDataSize = 0;
	int ret;

	if (inflateInit(&strm) != Z_OK) {
		// Error starting inflation
		free(data);
		return false;
	}

	unsigned int chunksSent = 0;
	while (len > 0) {
		if (len > ZLIB_CHUNK) {
			//Not the final input chunk
			
			memcpy(pInBuffer, (data + (chunksSent * ZLIB_CHUNK)), ZLIB_CHUNK);

			strm.next_in = pInBuffer;
			strm.avail_in = ZLIB_CHUNK;
			strm.next_out = pOutBuffer;
			strm.avail_out = ZLIB_CHUNK;

			if (inflate(&strm, Z_NO_FLUSH) != Z_OK) {
				// Error inflating

				inflateEnd(&strm);
				free(data);

				if (inflatedData != nullptr) free(inflatedData);
				return false;
			}

			// Copy new data to inflatedData
			if (inflatedData == nullptr) {
				inflatedData = (unsigned char*) malloc(strm.avail_out);
				memcpy(inflatedData, pOutBuffer, strm.avail_out);
			}
			else {
				inflatedDataTmp = (unsigned char*) malloc(inflatedDataSize + strm.avail_out);
				memcpy(inflatedDataTmp, inflatedData, inflatedDataSize);
				memcpy((inflatedDataTmp + inflatedDataSize), pOutBuffer, strm.avail_out);
				free(inflatedData);
				inflatedData = inflatedDataTmp;
			}

			len -= ZLIB_CHUNK;
			chunksSent++;
		}
		else {
			// Final input chunk

			memcpy(pInBuffer, (data + (chunksSent * ZLIB_CHUNK)), len);

			strm.next_in = pInBuffer;
			strm.avail_in = len;
			strm.next_out = pOutBuffer;
			strm.avail_out = ZLIB_CHUNK;

			ret = inflate(&strm, Z_NO_FLUSH);
			if ((ret != Z_OK) && (ret != Z_STREAM_END)) {
				// Error inflating

				inflateEnd(&strm);
				free(data);

				if (inflatedData != nullptr) free(inflatedData);
				return false;
			}

			// Copy new data to inflatedData
			if (inflatedData == nullptr) {
				inflatedData = (unsigned char*)malloc(strm.avail_out);
				memcpy(inflatedData, pOutBuffer, strm.avail_out);
			}
			else {
				inflatedDataTmp = (unsigned char*)malloc(inflatedDataSize + strm.avail_out);
				memcpy(inflatedDataTmp, inflatedData, inflatedDataSize);
				memcpy((inflatedDataTmp + inflatedDataSize), pOutBuffer, strm.avail_out);
				free(inflatedData);
				inflatedData = inflatedDataTmp;
			}

			break;
		}
	}

	// There may be more data to be output by inflate(), so we grab that until Z_STREAM_END if we don't have it already.
	while (ret != Z_STREAM_END) {
		strm.next_in = pInBuffer;
		strm.avail_in = 0;
		strm.next_out = pOutBuffer;
		strm.avail_out = ZLIB_CHUNK;

		ret = inflate(&strm, Z_NO_FLUSH);
		if (ret != Z_OK) {
			// Error inflating

			inflateEnd(&strm);
			free(data);

			if (inflatedData != nullptr) free(inflatedData);
			return false;
		}

		// Copy new data to inflatedData
		if (inflatedData == nullptr) {
			inflatedData = (unsigned char*)malloc(strm.avail_out);
			memcpy(inflatedData, pOutBuffer, strm.avail_out);
		}
		else {
			inflatedDataTmp = (unsigned char*)malloc(inflatedDataSize + strm.avail_out);
			memcpy(inflatedDataTmp, inflatedData, inflatedDataSize);
			memcpy((inflatedDataTmp + inflatedDataSize), pOutBuffer, strm.avail_out);
			free(inflatedData);
			inflatedData = inflatedDataTmp;
		}
	}

	// Clean up and exit
	inflateEnd(&strm);
	free(data);
	if (inflatedData != nullptr) free(inflatedData);
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

	FILE* exe = fopen(pFilename, "r");

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

	fread(buffer, fsize, 1, exe);
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
	unsigned char* zlibIn = (unsigned char*) malloc(ZLIB_CHUNK);
	unsigned char* zlibOut = (unsigned char*) malloc(ZLIB_CHUNK);

	// Settings Data Chunk
	unsigned char* data;
	unsigned int dataLength;
	InflateBlock(buffer, &pos, zlibIn, zlibOut, &data, &dataLength);

	//Cleaning up
	free(zlibIn);
	free(zlibOut);
	free(buffer);
	return true;
}
