#include <stdlib.h>
#include "CodeAction.hpp"
#include "StreamUtil.hpp"

CodeAction::CodeAction() {
	argCount = 0;
	args = NULL;
}

CodeAction::~CodeAction() {
	while (argCount) {
		argCount--;
		free(args[argCount]);
	}
	delete[] args;
}

bool CodeAction::read(const unsigned char* stream, unsigned int* pos)
{
	(*pos) += 8; // Skips version id and useless lib id
	actionID = ReadDword(stream, pos);
	(*pos) += 8; // Skips the useless "kind" variable and a flag for whether it can be relative
	question = ReadDword(stream, pos);
	(*pos) += 8; // Skips a flag for whether this action applies to something, and the useless "type" var
	(*pos) += ReadDword(stream, pos); // Function name?
	(*pos) += ReadDword(stream, pos); // Function code?

	// Delete the existing args, to avoid a memory leak just in case we ever call read() more than once for the same instance.
	while (argCount) {
		argCount--;
		free(args[argCount]);
	}
	delete[] args;

	argCount = ReadDword(stream, pos);
	if (argCount > 8) return false; // There's space for 8 args per action. If we try to read more than this, something's wrong.
	args = new char*[argCount];

	(*pos) += 36; // Skips a version id and all the arg types
	appliesToObject = ReadDword(stream, pos);
	relative = ReadDword(stream, pos);

	(*pos) += 4; // Another version id..

	unsigned int i;
	for (i = 0; i < argCount; i++) {
		args[i] = ReadString(stream, pos);
	}

	(*pos) += ((8 - i) * 5); // Skip unused arg strings. These should all be 1-length strings that say "0".
	not = ReadDword(stream, pos);
	return true;
}
