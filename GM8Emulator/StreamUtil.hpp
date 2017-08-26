#ifndef _A_STREAMUTIL_HPP_
#define _A_STREAMUTIL_HPP_

// Reads a dword from the given position in the byte stream
unsigned int ReadDword(const unsigned char* pStream, unsigned int* pPos);

// Reads a null-terminated string from the given position in the byte stream
// This allocates space for the string, so you should always give it to an object whose destructor will free it.
char* ReadString(const unsigned char* pStream, unsigned int* pPos);

// Reads a double from the given position in the byte stream
double ReadDouble(const unsigned char* pStream, unsigned int* pPos);

#endif
