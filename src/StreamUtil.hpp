#pragma once

// Reads a dword from the given position in the byte stream
unsigned int ReadDword(const unsigned char* pStream, unsigned int* pPos);

// Reads a null-terminated string from the given position in the byte stream
// This allocates space for the string, so you should always give it to an object whose destructor will free it.
// If the "len" ptr is set, the length of the read string will be output there.
char* ReadString(const unsigned char* pStream, unsigned int* pPos, unsigned int* pLen = nullptr);

// Reads a double from the given position in the byte stream
double ReadDouble(const unsigned char* pStream, unsigned int* pPos);

