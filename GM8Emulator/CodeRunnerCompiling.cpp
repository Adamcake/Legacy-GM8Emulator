#include "CodeRunner.hpp"
#include "CREnums.hpp"
#include "AssetManager.hpp"
#include <string>
#include <sstream>

//Removes all the comments from some code.
std::string removeComments(std::string input) {
	std::string inputNoComments = "";
	bool singleQuoteString = false;
	bool doubleQuoteString = false;
	bool singleLineComment = false;
	bool multiLineComment = false;

	for (unsigned int i = 0; i < input.size(); i++) {
		char thisChar = input.c_str()[i];

		if (i == input.size() - 1) {
			if ((!singleLineComment) && (!multiLineComment)) {
				inputNoComments += thisChar;
			}
		}
		else {
			char nextChar = input.c_str()[i + 1];

			if (thisChar == '/') {
				if (nextChar == '/' && (!multiLineComment) && (!singleQuoteString) && (!doubleQuoteString)) {
					singleLineComment = true;
					i++;
					continue;
				}
				else if (nextChar == '*' && (!singleLineComment) && (!singleQuoteString) && (!doubleQuoteString)) {
					multiLineComment = true;
					i++;
					continue;
				}
				else {
					if ((!singleLineComment) && (!multiLineComment)) {
						inputNoComments += thisChar;
					}
				}
			}

			else if (thisChar == '*') {
				if (nextChar == '/' && (!singleQuoteString) && (!doubleQuoteString)) {
					multiLineComment = false;
					i++;
				}
				else {
					inputNoComments += thisChar;
				}
			}

			else if (thisChar == '\'') {
				if ((!multiLineComment) && (!singleLineComment)) {
					if (!doubleQuoteString) {
						singleQuoteString = !singleQuoteString;
					}
					inputNoComments += thisChar;
				}
			}

			else if (thisChar == '"') {
				if ((!multiLineComment) && (!singleLineComment)) {
					if (!singleQuoteString) {
						doubleQuoteString = !doubleQuoteString;
					}
					inputNoComments += thisChar;
				}
			}

			else if (thisChar == 10 || thisChar == 13) {
				singleLineComment = false;
				inputNoComments += thisChar;
			}

			else {
				if ((!singleLineComment) && (!multiLineComment)) {
					inputNoComments += thisChar;
				}
			}
		}
	}

	return inputNoComments;
}

//Separates all the strings and constant numbers from a block of code into the _constants vector, replacing them with %n in the code (where n is their constant id)
//This function assumes there are no comments in the code.
std::string CodeRunner::substituteConstants(std::string input) {
	std::string inputNoStrings = "";
	std::string currentString = "";
	unsigned int currentSLen = 0;
	bool singleQuoteString = false;
	bool doubleQuoteString = false;

	for (unsigned int i = 0; i < input.size(); i++) {
		if (input[i] == '\'') {
			if (!doubleQuoteString) {
				if (singleQuoteString) {
					singleQuoteString = false;
					inputNoStrings += std::to_string(_RegConstantString(currentString.c_str(), currentSLen));
					inputNoStrings += "%";
					currentString = "";
					currentSLen = 0;
				}
				else {
					singleQuoteString = true;
					inputNoStrings += "%";
				}
			}
			else {
				currentString += input[i];
				currentSLen++;
			}
		}
		else if (input[i] == '"') {
			if (!singleQuoteString) {
				if (doubleQuoteString) {
					doubleQuoteString = false;
					inputNoStrings += std::to_string(_RegConstantString(currentString.c_str(), currentSLen));
					inputNoStrings += "%";
					currentString = "";
					currentSLen = 0;
				}
				else {
					doubleQuoteString = true;
					inputNoStrings += "%";
				}
			}
			else {
				currentString += input[i];
				currentSLen++;
			}
		}
		else {
			if (singleQuoteString || doubleQuoteString) {
				currentString += input[i];
				currentSLen++;
			}
			else {
				if (input[i] == '$') {
					std::string hexNum;
					i++;
					while ((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'a' && input[i] <= 'f') || (input[i] >= 'A' && input[i] <= 'F')) {
						hexNum += input[i];
						i++;
					}
					int v = 0;
					if (hexNum.size() > 0) {
						std::stringstream ss;
						ss << std::hex << hexNum;
						ss >> v;
					}
					inputNoStrings += "%" + std::to_string(_RegConstantDouble((double)v)) + "%";
				}
				else {
					inputNoStrings += input[i];
				}
			}
		}
	}

	if (singleQuoteString || doubleQuoteString) {
		inputNoStrings += _RegConstantString(currentString.c_str(), currentSLen);
	}

	return inputNoStrings;
}

unsigned int CodeRunner::_RegConstantDouble(double d) {
	for (unsigned int i = 0; i < _constants.size(); i++) {
		GMLType t = _constants[i];
		if (t.state == GML_TYPE_DOUBLE && t.dVal == d) {
			return i;
		}
	}
	GMLType t;
	t.state = GML_TYPE_DOUBLE;
	t.dVal = d;
	unsigned int ret = (unsigned int)_constants.size();
	_constants.push_back(t);
	return ret;
}

unsigned int CodeRunner::_RegConstantString(const char* c, unsigned int len) {
	for (unsigned int i = 0; i < _constants.size(); i++) {
		GMLType t = _constants[i];
		if (t.state == GML_TYPE_STRING && strcmp(c, t.sVal) == 0) {
			return i;
		}
	}
	GMLType t;
	t.state = GML_TYPE_STRING;
	t.sVal = (char*)malloc(len);
	memcpy(t.sVal, c, len);
	unsigned int ret = (unsigned int)_constants.size();
	_constants.push_back(t);
	return ret;
}

unsigned int CodeRunner::_RegField(const char * c, unsigned int len) {
	for (unsigned int i = 0; i < _fields.size(); i++) {
		if (strcmp(_fields[i], c) == 0) {
			return i;
		}
	}
	unsigned int ret = (unsigned int)_fields.size();
	char* field = (char*)malloc(len + 1);
	memcpy(field, c, len);
	field[len] = '\0';
	_fields.push_back(field);
	return ret;
}

bool CodeRunner::_makeVal(const char* exp, unsigned int len, unsigned char* out, std::vector<std::string>* session) {
	// If the length is 0 we can't make a VAL from this.
	if (!len) return false;

	// Next, check if this starts with %. If so, we need to point it to the const db.
	if (exp[0] == '%') {
		// Const db.
		out[0] = (unsigned char)(2 << 6);

		// Collect the string and turn it into an int.
		std::string ref;
		ref.reserve(len - 1);
		for (unsigned int pos = 1; pos < len; pos++) {
			if (exp[pos] == '%') {
				break;
			}
			else {
				ref += exp[pos];
			}
		}
		int iref = std::atoi(ref.c_str());

		out[2] = iref & 0xFF;
		out[1] = (iref & 0xFF00) >> 8;
		iref >>= 16;
		if (iref) {
			if (iref > 0x3F) {
				// This number is too big, and we have WAY too many constants. Return false.
				return false;
			}
			out[0] &= iref;
		}
		return true;
	}

	// Next see if we can make this into an absolute int.
	std::string strInt;
	strInt.reserve(len);
	bool num = true;
	bool endOfNum = false;
	for (unsigned int pos = 0; pos < len; pos++) {
		if (!endOfNum) {
			if (exp[pos] >= '0' && exp[pos] <= '9') {
				strInt += exp[pos];
			}
			else {
				if (exp[pos] == ' ' || exp[pos] == 10 || exp[pos] == 13) {
					endOfNum = true;
				}
				else {
					num = false;
					break;
				}
			}
		}
		else {
			if (exp[pos] == ' ' || exp[pos] == 10 || exp[pos] == 13) {
				continue;
			}
			else {
				num = false;
				break;
			}
		}
	}
	if (num) {
		// We can try to write this value as an absolute int
		int value = std::atoi(strInt.c_str());
		if (value < 0x400000) {
			// This value can safely be written as an absolute int
			out[0] = (unsigned char)(1 << 6);

			out[2] = (unsigned char)(value & 0xFF);
			out[1] = (unsigned char)((value >> 8) & 0xFF);
			out[0] |= (unsigned char)(value >> 16); // This is safe because we verified that the top 10 bits are unused
			return true;
		}
	}

	// Nothing else worked so we have to register this as an expression.
	unsigned char* o;
	if (!_CompileExpression(exp, &o)) {
		return false;
	}
	unsigned int ret = (unsigned int)_codeObjects.size();
	_codeObjects.push_back(CRCodeObject());
	_codeObjects[ret].compiled = o;
	_codeObjects[ret].question = true;

	if (ret >= 0x400000) {
		// How do you even have this many code objects registered? Unfortunately we can't encode more than 22 bits into a VAL.
		return false;
	}
	out[0] = (unsigned char)(3 << 6);
	out[2] = (unsigned char)(ret & 0xFF);
	out[1] = (unsigned char)((ret >> 8) & 0xFF);
	out[0] |= (unsigned char)(ret >> 16); // This is safe because we verified that the top 10 bits are unused
	return true;
}

// Helper function to tell us if a char is alphanumeric (which in this context also includes underscores)
bool isAlphanumeric(char c) {
	if (c >= 'a' && c <= 'z') return true;
	if (c >= 'A' && c <= 'Z') return true;
	if (c >= '0' && c <= '9') return true;
	if (c == '_') return true;
	return false;
}

// Helper function that gets the first non-whitespace character after a given pos, moving the pos marker.
void findFirstNonWhitespace(std::string input, unsigned int* pos) {
	while (input[*pos] < 0x20) (*pos)++;
}

// Helper function that gets the first alphanumeric word after a given pos, also moving the pos marker.
std::string getWord(std::string input, unsigned int* pos) {
	findFirstNonWhitespace(input, pos);
	std::string ret;

	while ((*pos) < input.size()) {
		char c = input[*pos];
		if (isAlphanumeric(c)) {
			ret += c;
			(*pos)++;
		}
		else {
			break;
		}
	}

	return ret;
}

// Helper function to get the contents of some brackets, also moving the pos marker past them.
// This function takes nested brackets into account rather than just stopping at the first close-bracket.
std::string getBracketContents(std::string input, unsigned int* pos, char start = '(', char end = ')') {
	if (input[*pos] != start) return ""; // ?
	(*pos)++;

	std::string ret;
	unsigned int depth = 1;
	while (depth) {
		if (input[*pos] == start) depth++;
		else if (input[*pos] == end) depth--;
		if(depth) ret += input[*pos];
		(*pos)++;
	}
	return ret;
}

// Helper function to get what type a variable name is (local, field, etc)
CRVarType CodeRunner::getVarType(std::string name, std::vector<std::string>* locals, unsigned int* index) {

	// Game values have highest precedence
	for (unsigned int i = 0; i < _readOnlyGameValueNames.size(); i++) {
		if (strcmp(_readOnlyGameValueNames[i], name.c_str()) == 0) {
			if (index) (*index) = i;
			return VARTYPE_GAME_RONLY;
		}
	}
	for (unsigned int i = 0; i < _RWGameValueNames.size(); i++) {
		if (strcmp(_RWGameValueNames[i], name.c_str()) == 0) {
			if (index) (*index) = i;
			return VARTYPE_GAME_RW;
		}
	}

	// Next, instance variables
	for (unsigned int i = 0; i < _instanceVarNames.size(); i++) {
		if (strcmp(_instanceVarNames[i], name.c_str()) == 0) {
			if (index) (*index) = i;
			return VARTYPE_INSTANCE;
		}
	}

	// Now special names.
	for (unsigned int i = 0; i < _specialVarNames.size(); i++) {
		if (strcmp(_specialVarNames[i], name.c_str()) == 0) {
			if (index) (*index) = i;
			return VARTYPE_SPECIAL;
		}
	}

	// Next local vars - but only if we've been given a list
	if (locals) {
		for (unsigned int i = 0; i < locals->size(); i++) {
			if ((*locals)[i] == name) {
				if (index) (*index) = i;
				return VARTYPE_LOCAL;
			}
		}
	}

	// If none of the above, it must be a field, let's register it
	unsigned int ix = _RegField(name.c_str(), (unsigned int)name.size());
	if (index) (*index) = ix;
	return VARTYPE_FIELD;
}

// Helper function to get a set method from a given point in a string. Also moves the pos.
// Returns false if no set method could be found (ie. game should exit.)
bool getSetMethod(std::string input, unsigned int* pos, CRSetMethod* method) {
	// Equals sign has precedence
	if (input[*pos] == '=') {
		(*pos)++;
		(*method) = SM_ASSIGN;
		return true;
	}

	// If the operator isn't something immediately followed by an equals sign, this isn't a set method at all
	if (input[(*pos) + 1] != '=') {
		return false;
	}

	// Figure out which one it is
	CRSetMethod ret;
	switch (input[*pos]) {
		case '+':
			ret = SM_ADD;
			break;
		case '-':
			ret = SM_SUBTRACT;
			break;
		case '*':
			ret = SM_MULTIPLY;
			break;
		case '/':
			ret = SM_DIVIDE;
			break;
		case '|':
			ret = SM_BITWISE_OR;
			break;
		case '&':
			ret = SM_BITWISE_AND;
			break;
		case '^':
			ret = SM_BITWISE_XOR;
			break;
		default:
			return false;
	}

	(*pos) += 2;
	(*method) = ret;
	return true;
}

// Helper function to check if a string starts with another string.
bool StartsWith(std::string str, std::string arg) {
	if (str.size() < arg.size()) return false;
	return !(strcmp(str.substr(0, arg.size()).c_str(), arg.c_str()));
}


// Helper function that parses an expression from code and returns a VAL reference, also moving the pos.
bool CodeRunner::getExpression(std::string input, unsigned int* pos, std::vector<std::string>* locals, unsigned char* outVal) {
	findFirstNonWhitespace(input, pos);
	if (!((*pos) < input.size())) return false;

	std::vector<std::string> ANOperators = { "and", "xor", "mod", "div", "or" };
	std::vector<std::string> operators = { "^^", "<<", ">>", "&&", "||", "==", "!=", "<=", ">=", "=", "<", ">", "+", "-", "*", "/", "&", "|", "^" };

	unsigned int startPos = *pos;

	while ((*pos) < input.size()) {
		bool checkDotsAndBrackets = true;
		findFirstNonWhitespace(input, pos);
		if (!((*pos) < input.size())) break;

		if (input[*pos] == '!' || input[*pos] == '~' || input[*pos] == '-' || input[*pos] == '$') {
			(*pos)++;
			continue;
		}

		if ((!strcmp(input.substr((*pos), 3).c_str(), "not")) && (!isAlphanumeric(input[(*pos) + 3]))) {
			(*pos) += 3;
			continue;
		}

		if (input[*pos] == '(') {
			getBracketContents(input, pos);
			if ((*pos) >= input.size()) break;
		}
		else if (input[*pos] == '%') {
			(*pos)++;
			while (input[*pos] != '%') (*pos)++;
			(*pos)++;
			if ((*pos) >= input.size()) break;
		}
		else if (input[*pos] >= '0' && input[*pos] <= '9') {
			checkDotsAndBrackets = false;
			while ((input[*pos] >= '0' && input[*pos] <= '9') || input[*pos] == '.') {
				(*pos)++;
			}
		}
		else if (isAlphanumeric(input[*pos])) {
			while (isAlphanumeric(input[*pos])) {
				(*pos)++;
				if ((*pos) >= input.size()) break;
			}
			if ((*pos) >= input.size()) break;

			findFirstNonWhitespace(input, pos);
			if ((*pos) >= input.size()) break;

			if (input[*pos] == '[') {
				getBracketContents(input, pos, '[', ']');
				findFirstNonWhitespace(input, pos);
				if ((*pos) >= input.size()) break;
			}
			else if (input[*pos] == '(') {
				getBracketContents(input, pos, '(', ')');
				findFirstNonWhitespace(input, pos);
				if ((*pos) >= input.size()) break;
			}
		}

		while (checkDotsAndBrackets) {
			findFirstNonWhitespace(input, pos);
			if (input[*pos] == '.') {
				(*pos)++;

				while (isAlphanumeric(input[*pos])) {
					pos++;
					if ((*pos) >= input.size()) break;
				}

				findFirstNonWhitespace(input, pos);
				if ((*pos) >= input.size()) break;

				if (input[*pos] == '[') {
					getBracketContents(input, pos, '[', ']');
					if ((*pos) >= input.size()) break;
				}
			}
			else {
				break;
			}
		}

		//We've skipped over a single variable. So now we determine whether there's an operator after it. If not, the expression ends here.

		findFirstNonWhitespace(input, pos);
		if ((*pos) >= input.size()) break;

		std::string maybeOperator = input.substr(*pos);
		bool matchFound = false;

		for (std::string op : ANOperators) {
			if (StartsWith(maybeOperator, op) && (!isAlphanumeric(maybeOperator[op.size()]))) {
				matchFound = true;
				(*pos) += (unsigned int)op.size();
				break;
			}
		}

		if (matchFound) continue;

		for (std::string op : operators) {
			if (StartsWith(maybeOperator, op)) {
				matchFound = true;
				(*pos) += (unsigned int)op.size();
				break;
			}
		}

		if (!matchFound) break;

	}

	if ((*pos) == startPos) return false;

	std::string expression = input.substr(startPos, (*pos) - startPos);
	if (!_makeVal(expression.c_str(), (unsigned int)expression.size(), outVal, locals)) {
		return false;
	}

	return true;
}


bool CodeRunner::_CompileCode(const char* str, unsigned char** outHandle, std::vector<std::string>* session) {

	// We only have to remove comments and substitute strings if we're not already in an existing session.
	std::string code;
	if(!session)code = substituteConstants(removeComments(str));
	else code = std::string(str);

	// If we've been passed an existing session then we use its locals, otherwise we make our own.
	std::vector<std::string>* locals;
	if (!session) locals = new std::vector<std::string>;
	else locals = session;

	unsigned int pos = 0;
	std::vector<unsigned char> output;
	output.reserve(256); // Prevents a lot of realloc calls when there's a small amount of memory in the vector

	// We loop through, pulling commands until there are no more.
	while (pos < code.size()) {
		// Get the first word at the current position. Word may be something like "var", "while", or a script or function name, and so on.
		std::string firstWord = getWord(code, &pos);

		// Exit if we're at the end of the string
		if (pos >= code.size()) {
			break;
		}

		// Based on the first word we're going to decide what type of command this is.

		if (firstWord == "exit") {
			// This keyword means exit the script, so we compile this into an "01" instruction.
			output.push_back(OP_EXIT);
			findFirstNonWhitespace(code, &pos);
			if (code[pos] == ';') pos++;
			continue;
		}
		else if (firstWord == "var") {
			// The "var" keyword tells us to bind some var names to local variables. This doesn't incur any actual bytecode in itself.
			while (true) {
				// Get the var name and put it in our list
				std::string firstWord = getWord(code, &pos);
				locals->push_back(firstWord);

				// Check if there's another var name on the same line, these must be comma-separated
				findFirstNonWhitespace(code, &pos);
				if (code[pos] == ',') {
					// Get another var name
					pos++;
					continue;
				}
				else {
					// No more var names
					if (code[pos] == ';') pos++;
					break;
				}
			}
			continue;
		}
		else if (firstWord == "while") {
			// "while" generates a loop, which translates to tests and jumps in bytecode.
			// TBD
		}
		else if (firstWord == "do") {
			// "do" generates a loop in which the first iteration is always run. It can only be terminated by a "while" or "until" statement.
			// TBD
		}
		else if (firstWord == "for") {
			// "for" generates a loop in a rather roundabout way. This involves some tests and jumps.
			// TBD
		}
		else if (firstWord == "if") {
			// "if" incurs a simple test to see whether the following expression evaluates to true.
			// TBD
		}
		else if (firstWord == "with") {
			// "with" indicates a change in the "self" and "other" variables for the contained code block.
			// TBD
		}
		else if (firstWord == "else") {
			// We should never find an "else" here. Compiling fails.
			if (!session) delete locals;
			return false;
		}
		else {
			// If this doesn't start with any keywords, then it's either an assignment or a script call.
			findFirstNonWhitespace(code, &pos);
			if (firstWord.size() > 0 && code[pos] == '(') {
				// It's a script or internal function call. firstWord is the script/function name.
				// User scripts have precedence over internal functions, so first check if there's a script with this name.
				Script* scr = NULL;
				unsigned int scriptId;
				for (scriptId = 0; scriptId < _assetManager->GetScriptCount(); scriptId++) {
					Script* s = _assetManager->GetScript(scriptId);
					if (s->exists) {
						if (strcmp(s->name, firstWord.c_str()) == 0) {
							scr = s;
							break;
						}
					}
				}

				if (scr) {
					// User script.
					output.push_back(OP_RUN_SCRIPT);
					output.push_back((unsigned char)(scriptId&0xFF)); // Script id, little endian
					output.push_back((unsigned char)((scriptId & 0xFF00) >> 8));
				}
				else {
					// Built-in function.
					output.push_back(OP_RUN_INTERNAL_FUNC);

					// First resolve the function name to an internal function id.
					unsigned int funcId;
					for (funcId = 0; funcId < _INTERNAL_FUNC_COUNT; funcId++) {
						if (strcmp(_internalFuncNames[funcId], firstWord.c_str()) == 0) {
							break;
						}
					}
					if (funcId == _INTERNAL_FUNC_COUNT) {
						// We didn't find this as an internal function, that means we can't compile this.
						if (!session) delete locals;
						return false;
					}
					// First two bytes indicate func id
					output.push_back((unsigned char)(funcId & 0xFF));
					output.push_back((unsigned char)((funcId >> 8) & 0xFF));
				}

				// We'll use this as a reference later for setting the number of args after they've all been parsed.
				unsigned int argCPos = (unsigned int)output.size();
				output.push_back(0);

				// Args
				unsigned char argCount = 0;
				unsigned char valBuffer[3];

				pos++;
				findFirstNonWhitespace(code, &pos);
				if (code[pos] != ')') {
					while (true) {
						findFirstNonWhitespace(code, &pos);
						std::string arg;
						arg.reserve(16);
						while (code[pos] != ',' && code[pos] != ')') {
							arg += code[pos];
							if (code[pos] == '(') arg += getBracketContents(code, &pos, '(', ')') + ')';
							else if (code[pos] == '[') arg += getBracketContents(code, &pos, '[', ']') + ']';
							else pos++;
						}
						if (!_makeVal(arg.c_str(), (unsigned int)arg.size(), valBuffer, locals)) {
							// Error reading expression.
							if (!session) delete locals;
							return false;
						}
						output.push_back(valBuffer[0]);
						output.push_back(valBuffer[1]);
						output.push_back(valBuffer[2]);
						argCount++;

						if (code[pos] == ')') {
							break;
						}
						else {
							pos++;
						}
					}
				}
				output[argCPos] = argCount;
				pos++;
				findFirstNonWhitespace(code, &pos);
				if (code[pos] == ';') pos++;
				continue;
			}
			else {
				// This is an assignment. First, our bytecode needs to dereference the correct instance to assign to.
				std::string nextWord;

				if (firstWord.size() == 0) {
					// A line of code that starts with a bracket is a special case. It's always an assignment.
					// It means we need to deref the expression inside the brackets.
					if (code[pos] != '(') {
						// Invalid GML.
						if (!session) delete locals;
						return false;
					}

					std::string exp = getBracketContents(code, &pos);
					findFirstNonWhitespace(code, &pos);
					if (code[pos] != '.') {
						// Invalid GML again.
						if (!session) delete locals;
						return false;
					}
					pos++;

					unsigned char val[3];
					if (!_makeVal(exp.c_str(), (unsigned int)exp.size(), val, locals)) {

					}
					output.push_back(OPERATOR_DEREF);
					output.push_back(val[0]);
					output.push_back(val[1]);
					output.push_back(val[2]);

					nextWord = getWord(code, &pos);
				}
				else {
					nextWord = firstWord;
				}

				// Arrays and derefs can be chained infinitely (eg: self.a[0].a[0].a.a[0].a...=1) So we loop them.
				bool anyDerefs = false;
				std::string derefExpression;
				std::string arrayIndex;
				bool array;
				while (true) {
					array = false;
					arrayIndex.clear();

					findFirstNonWhitespace(code, &pos);

					if (code[pos] == '[') {
						arrayIndex = getBracketContents(code, &pos, '[', ']');
						array = true;
						findFirstNonWhitespace(code, &pos);
					}

					if (code[pos] == '.') {
						// Push onto deref expression and loop again
						if (anyDerefs) derefExpression += '.';
						derefExpression += nextWord;
						if (array) derefExpression += "[" + arrayIndex + "]";

						pos++;
						nextWord = getWord(code, &pos);
						anyDerefs = true;
					}
					else {
						// Write the deref expression if one is needed, then go to the next part
						if (anyDerefs) {
							unsigned char val[3];
							_makeVal(derefExpression.c_str(),(unsigned int)derefExpression.size(), val, locals);
							output.push_back(OP_DEREF);
							output.push_back(val[0]);
							output.push_back(val[1]);
							output.push_back(val[2]);
						}
						break;
					}
				}

				// Now the references are set correctly, so we just need to write an assignment operator: set local var, set game value, set instance variable, or set field.
				// The variable name is stored in nextWord so let's figure out which one to do.
				unsigned int varIx;
				CRVarType type = getVarType(nextWord, (anyDerefs ? NULL : locals), &varIx); // Only send the locals if there are no derefs, cause otherwise the variable can't be local
				switch (type) {
					case VARTYPE_LOCAL:
						if (!array) {
							output.push_back(OP_SET_LOCAL_VAR);
							output.push_back((unsigned char)(varIx & 0xFF));
						}
						else {
							output.push_back(OP_SET_LOCAL_ARRAY);
							output.push_back((unsigned char)(varIx & 0xFF));
							unsigned char val[3];
							if (!_makeVal(arrayIndex.c_str(), (unsigned int)arrayIndex.size(), val, locals)) {
								// Error in GML
								if (!session) delete locals;
								return false;
							}
							output.push_back(val[0]);
							output.push_back(val[1]);
							output.push_back(val[2]);
						}
						break;
					case VARTYPE_FIELD:
						if (array) {
							output.push_back(OP_SET_ARRAY);
							unsigned char val[3];
							if (!_makeVal(arrayIndex.c_str(), (unsigned int)arrayIndex.size(), val, locals)) {
								// Error in GML
								if (!session) delete locals;
								return false;
							}
							output.push_back(val[0]);
							output.push_back(val[1]);
							output.push_back(val[2]);
						}
						else {
							output.push_back(OP_SET_FIELD);
						}
						output.push_back((unsigned char)(varIx & 0xFF));
						output.push_back((unsigned char)((varIx >> 8) & 0xFF));
						break;
					case VARTYPE_INSTANCE:
						if (array != (bool)(varIx == IV_ALARM)) { // if instance var is alarm and an array isn't used, OR it's not alarm and an array is used
							// Invalid instance var
							if (!session) delete locals;
							return false;
						}
						output.push_back(OP_SET_INSTANCE_VAR);
						output.push_back((unsigned char)(varIx & 0xFF));
						unsigned char val[3];
						if (array) {
							_makeVal(arrayIndex.c_str(), (unsigned int)arrayIndex.size(), val, locals);
						}
						output.push_back(val[0]);
						output.push_back(val[1]);
						output.push_back(val[2]);
						break;
					case VARTYPE_GAME_RONLY:
						output.push_back(OP_SET_GAME_VALUE);
						output.push_back((unsigned char)varIx);
						break;
					default:
						if (!session) delete locals;
						return false;
				}

				// No matter what OP type we wrote, the rest is the same. Next we need to work out the SET METHOD.
				findFirstNonWhitespace(code, &pos);
				CRSetMethod method;
				if (!getSetMethod(code, &pos, &method)) {
					// Invalid gml
					if (!session) delete locals;
					return false;
				}
				output.push_back(method);

				// Now we just have to get the VAL to assign.
				unsigned char val[3];
				if (! getExpression(code, &pos, locals, val)) {
					if (!session) delete locals;
					return false;
				}
				output.push_back(val[0]);
				output.push_back(val[1]);
				output.push_back(val[2]);

				if (anyDerefs) {
					// Write a "reset deref" instruction after we're done here.
					output.push_back(OP_RESET_DEREF);
				}
				if (code[pos] == ';') pos++;
				continue;
			}
		}
	}

	// Our bytecode must be terminated by an 01. So if the last instruction doesn't happen to be an 01, we'll put one there.
	if (output.size() == 0) {
		output.push_back(OP_EXIT);
	}
	else if (output[output.size() - 1] != OP_EXIT) {
		output.push_back(OP_EXIT);
	}

	// Write the finished operator list to the output pointer
	(*outHandle) = (unsigned char*)malloc(output.size());
	memcpy((*outHandle), output._Myfirst(), output.size());

	// Only delete the session if we made it
	if (!session) delete locals;

	return true;
}





bool CodeRunner::_CompileExpression(const char* str, unsigned char** outHandle, std::vector<std::string>* session) {
	// We only have to remove comments and substitute strings if we're not already in an existing session.
	std::string code;
	if (!session)code = substituteConstants(removeComments(str));
	else code = std::string(str);
	unsigned int pos = 0;

	// Trim any whitespace from the start and end of the input string
	unsigned int start = code.find_first_not_of({ ' ', 10, 13, '\0' });
	unsigned int end = code.find_last_not_of({ ' ', 10, 13, '\0' });
	code = code.substr(start, (end - start) + 1);

	// Remove any bracket pairs that are surrounding the whole expression
	while (code[0] == '(' && code.back() == ')') {
		std::string newCode = code.substr(1, code.size() - 2);
		if (newCode == getBracketContents(code, &pos)) {
			code = newCode;
		}
		else {
			break;
		}
	}
	pos = 0;

	// Parse any modifiers
	bool negative = false; //-
	bool not = false; //!
	bool tilde = false; //~
	bool mods = true;
	while (mods) {
		switch (code[pos]) {
			case '-':
				negative = !negative;
				break;
			case '!':
				not = !not;
				break;
			case '~':
				tilde = !tilde;
				break;
			default:
				mods = false;
				break;
		}
	}

	// Setup output buffer
	std::vector<unsigned char> output;

	// Now we have to figure out a VAR.
	
	std::string word = getWord(code, &pos);
	if (word.size() == 0 && code[pos] != '.') {
		// Expression doesn't start alphanumerically, so it must be brackets
		if (code[pos] != '(') return false;

		// Get bracket contents, turn it into a VAL, make this VAR point to that VAL, and move on
		std::string content = getBracketContents(code, &pos);
		unsigned char val[3];
		if (!_makeVal(content.c_str(), (unsigned int)content.size(), val, session)) {
			return false;
		}
		output.push_back(EVTYPE_VAL);
		output.push_back(val[0]);
		output.push_back(val[1]);
		output.push_back(val[2]);
	}
	else {
		findFirstNonWhitespace(code, &pos);

		// First, check if this is a number
		if ((word[0] >= '0' && word[0] <= '9') || word[0] == '.') {
			// TODO
		}
		else {
			// Next, check if this is a constant
			if (word[0] == '%') {
				// TODO
			}
			else {
				// Find out if this is a script/function or variable
				if (code[pos] == '(') {
					// It'a a script/function
					Script* scr = NULL;
					unsigned int scriptId;
					for (scriptId = 0; scriptId < _assetManager->GetScriptCount(); scriptId++) {
						Script* s = _assetManager->GetScript(scriptId);
						if (s->exists) {
							if (strcmp(s->name, word.c_str()) == 0) {
								scr = s;
								break;
							}
						}
					}

					if (scr) {
						// User script
						output.push_back(EVTYPE_SCRIPT);
						output.push_back((unsigned char)(scriptId & 0xFF)); // Script id, little endian
						output.push_back((unsigned char)((scriptId & 0xFF00) >> 8));
					}
					else {
						// Internal function
						output.push_back(OP_RUN_INTERNAL_FUNC);

						// First resolve the function name to an internal function id.
						unsigned int funcId;
						for (funcId = 0; funcId < _INTERNAL_FUNC_COUNT; funcId++) {
							if (strcmp(_internalFuncNames[funcId], word.c_str()) == 0) {
								break;
							}
						}
						if (funcId == _INTERNAL_FUNC_COUNT) {
							return false;
						}
						output.push_back((unsigned char)(funcId & 0xFF)); // Function id, little endian
						output.push_back((unsigned char)((funcId & 0xFF00) >> 8));
					}

					// Push arg count and save its position for later

					unsigned int argCPos = (unsigned int)output.size();
					output.push_back(0);

					// Now parse args
					unsigned char argCount = 0;
					unsigned char valBuffer[3];

					pos++;
					findFirstNonWhitespace(code, &pos);
					if (code[pos] != ')') {
						while (true) {
							findFirstNonWhitespace(code, &pos);
							std::string arg;
							arg.reserve(16);
							while (code[pos] != ',' && code[pos] != ')') {
								arg += code[pos];
								if (code[pos] == '(') arg += getBracketContents(code, &pos, '(', ')') + ")";
								else if (code[pos] == '[') arg += getBracketContents(code, &pos, '[', ']') + "]";
								else pos++;
							}
							if (!_makeVal(arg.c_str(), (unsigned int)arg.size(), valBuffer, session)) {
								return false;
							}
							output.push_back(valBuffer[0]);
							output.push_back(valBuffer[1]);
							output.push_back(valBuffer[2]);
							argCount++;

							if (code[pos] == ')') {
								break;
							}
							else {
								pos++;
							}
						}
					}
					output[argCPos] = argCount;
					pos++;
				}
				else {
					// TODO HERE: CHECK IF IT'S A GML CONST OR ASSET NAME (including constants?)

					// It's a "variable get". Now figure out if this word has an array index at the end of it.

					bool array = false;
					std::string arrayIndex;
					findFirstNonWhitespace(code, &pos);
					if (code[pos] == '[') {
						array = true;
						arrayIndex = getBracketContents(code, &pos, '[', ']');
					}

					// Get what type of variable we've been given
				}
			}
		}
	}

	// Write the finished list to the output pointer
	(*outHandle) = (unsigned char*)malloc(output.size());
	memcpy((*outHandle), output._Myfirst(), output.size());

	return true;
}