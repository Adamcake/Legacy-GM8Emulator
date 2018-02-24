#include <stdlib.h>
#include <string.h>
#include "StreamUtil.hpp"

unsigned int ReadDword(const unsigned char* pStream, unsigned int* pPos) {
	unsigned int val = pStream[(*pPos)] + (pStream[(*pPos) + 1] << 8) + (pStream[(*pPos) + 2] << 16) + (pStream[(*pPos) + 3] << 24);
	(*pPos) += 4;
	return val;
}

double ReadDouble(const unsigned char* pStream, unsigned int* pPos) {
	double val = *(double*)(pStream + (*pPos));
	(*pPos) += 8;
	return val;
}

char* ReadString(const unsigned char* pStream, unsigned int* pPos, unsigned int* pLen) {
	unsigned int length = ReadDword(pStream, pPos);
	char* str = (char*)malloc(length + 1);
	memcpy(str, (pStream + *pPos), length);
	str[length] = '\0';
	(*pPos) += length;
	if (pLen != nullptr) {
		(*pLen) = length;
	}
	return str;
}