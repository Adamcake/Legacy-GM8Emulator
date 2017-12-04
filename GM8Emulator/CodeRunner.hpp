#ifndef _A_CODERUNNER_HPP_
#define _A_CODERUNNER_HPP_
class AssetManager;
class InstanceList;
struct Instance;
#include <vector>
#include <map>
#include <stack>
#include <string>

typedef unsigned int CodeObject;
enum CRGameVar;
enum CRVarType;
enum CROperator;

/*
The runner compiles GML into an instruction format that is based on the format of x86 CPU instructions, except massively cut down. This is (vaguely) documented here, for now.

VAL:
Wherever "VAL" is used below, it refers to a 3-byte structure where the first 2 bits indicates the nature of the value, and the following 22 bits is the value itself.
The bytes for the latter part are big-endian, ie. the 6 bits in the first byte are only used in the case of a number too large for 2 bytes.
Byte 1 can have the following values:
- 0: top stack variable (bytes discarded)
- 1: absolute integer (bytes will be parsed into signed int)
- 2: const reference (bytes are a const db reference)
- 3: expression (bytes are an expression reference, expression will be evaluated)

The highest value that can be expressed by a VAL is 4194303. Having more than 4194303 strings in the application will crash it (although it would probably run out of memory before then anyway.)
If you wish to have an absolute integer higher than 4194303 in a VAL, you must use the const db or an expression.


ID:
Wherever "ID" is used below, it refers to a signed integer that can be used as an instance id. Like in GML, values between 0 and 100000 (inclusive) will be treated as object IDs
and will act on the first instance with that object_index. Values above 100000 will be treated as an instance id.
There are also magic numbers that can be used as ID, which are -1 (self), -2 (other), -3 (all), -4 (noone) and -5 (global).


SET METHOD:
Wherever "SET METHOD" is used below, it refers to a byte which tells the runner which type of assignment to use when setting a variable.
0: =
1: +=
2: -=
3: *=
4: /=
5: |=
6: &=
7: ^=

========

INSTRUCTIONS:

00: NOP (Means "do nothing")
Followed by 0 bytes. 

01: Exit (Means "stop running current code block" - the compiled code must be terminated by this)
Followed by 0 bytes. 

02: Set game value (room_speed, health, etc)
Followed by 8 bytes. The first indicates the game value to set, the following 3 indicate a VAL for the array index if applicable - otherwise discarded.
The following byte indicates the SET METHOD and the final 3 are a VAL.

03: Set instance variable (eg player.x)
Followed by 8 bytes. The first indicates the instance var id, the following 3 indicate the array index if this instance variable is an array - otherwise discarded.
The following byte indicates the SET METHOD and the final 3 are a VAL.
This will set the indicated instance variable of whatever is stored in the dereferencing buffer (default is self.)

04: Set field (eg self.abc)
Followed by 6 bytes. The first 2 indicate the field number, the following byte indicates the SET METHOD and the final 3 are a VAL.
This will set the indicated field of whatever is stored in the dereferencing buffer (default is self.)

05: Set array (eg self.abc[1])
Followed by 9 bytes. The first 2 indicate the array number, the following 3 are a VAL indicating the array index,
the following byte indicates the SET METHOD and the final 3 are the VAL for what to set it to.
This will set the indicated array index of whatever is stored in the dereferencing buffer (default is self.)

06: Bind local vars (eg "var a;")
Followed by (1+2n) bytes. The first indicates the number of fields to bind.

07: Dereference
Followed by 3 bytes indicating a VAL. This will evaluate the VAL into an ID, dereference that ID into an instance pointer, and store it in the deref buffer.

08: Reset Deref Buffer
Followed by 0 bytes. Resets the deref buffer to the "self" of the current context.

09: Run an internal function (instance_create, room_goto, etc)
Followed by a (3+3a) bytes. The first two bytes indicate the internal function. The following byte indicates the number of arguments following. Following that,
each argument is indicated by 3 bytes, each of which is a VAL.

0A: Run script
Followed by a (3+3a) bytes. The first two bytes indicate a script index. The following byte indicates the number of arguments following. Following that,
each argument is indicated by 3 bytes, each of which is a VAL.

0B: Test a value (next instruction is skipped if the VAL evaluates to false)
Followed by 3 bytes indicating the VAL to test.

0C: Test a value for falseness (next instruction is skipped if the VAL evaluates to true)
Followed by 3 bytes indicating the VAL to test.

0D: Test two values for equality (next instruction is skipped if the VALs do not match)
Followed by 6 bytes indicating the two VALs to test.

0E: Else (ie. the following op only gets run if the previous one was skipped by a test)
Followed by 0 bytes.

0F: Change context
Followed by 4 bytes indicating the target ID to use as "self". "other" will be set to the previous "self". The current state before changing will be pushed onto a session stack.

10: Revert context
Followed by 0 bytes. Pops values of "self" and "other" from the stack mentioned above.

11: Set top stack integer
Followed by 3 bytes indicating VAL.

12: Jump ahead short
Followed by 1 byte. The byte indicates how many instructions to jump ahead AFTER that byte.

13: Jump ahead long
Followed by 3 bytes. The bytes indicate how many instructions to jump ahead AFTER those bytes.

14: Jump back short
Followed by 1 byte. The byte indicates how many instructions to jump back AFTER that byte (ie. the distance must include the 2 bytes used by this instruction.)

15: Jump back long
Followed by 3 bytes. The bytes indicate how many instructions to jump back AFTER those bytes (ie. the distance must include the 4 bytes used by this instruction.)

16: Push onto stack
Followed by 0 bytes.

17: Pop from stack
Followed by 0 bytes.

18: Return
Followed by 3 bytes indicating a VAL. (Writes to the return buffer and exits.)


========

EXPRESSIONS:
An expression is some GML that gets compiled and can later be evaluated, returning a GMLType. It's formatted as:
VAR [OPERATOR VAR [OPERATOR VAR [OPERATOR VAR...]] OP
For example, in the GML line "player.x = mouse_x - 1", there is an expression: "mouse_x - 1". mouse_x is a VAR, 1 is a VAR, and - is an OP. 
The final OP must be a "STOP" to terminate the expression. OP and VAR are defined as:


"OPERATOR" refers to an operator which acts on two VARs (the LHS and RHS). These can have the following values:
00: STOP (end of expression)
01: +
02: -
03: *
04: /
05: mod
06: div
07: == (can also be "=" in gml expression)
08: !=
09: <
0A: <=
0B: >
0C: >=
0D: |
0E: || (can also be "or" in gml)
0F: &
10: && (can also be "and" in gml)
11: ^
12: ^^ (can also be "xor" in gml)
13: <<
14: >>
15: .
// TODO: Figure out precedence order and sort these as such.


"VAR" here refers to a variable in an expression. Like with instructions, the first byte indicates what follows it.
The BOTTOM 4 BITS of the byte reference the value table below. The TOP 4 BITS are a modifier bitmask.

VALUE TABLE:

00: Unused

01: VAL
Followed by 3 bytes which are interpreted as a VAL.

02: Get game value (room_speed, health, etc)
Followed by 4 bytes, the first indicates which game value to get and the following 3 are a VAL for the array index if applicable - otherwise discarded.

03: Get field (eg self.abc)
Followed by 2 bytes which indicate the field number.
This will read from the instance in the expression dereference buffer (default is self.)

04: Get array (eg self.abc[1])
Followed by 5 bytes. The first 2 indicate the array number, the final 3 are a VAL indicating the array index.
This will read from the instance in the expression dereference buffer (default is self.)

05: Get instance variable (eg sprite_index)
Followed by 4 bytes, the first is the instance variable id and the last 3 are an array index VAL if applicable (discard if not.)
This will read from the instance in the expression dereference buffer (default is self.)

06: Get result of internal function
Followed by a (3+3a) bytes. The first two bytes indicate the internal function. The following byte indicates the number of arguments following. Following that,
each argument is indicated by 3 bytes, each of which is a VAL.

07: Get result of script
Followed by a (3+3a) bytes. The first two bytes indicate a script index. The following byte indicates the number of arguments following. Following that,
each argument is indicated by 3 bytes, each of which is a VAL.

08: Get top stack value
Followed by 0 bytes. Gets the value from the internal stack.


VAR MODIFIERS:

Fifth bit: the following VAR is negative - eg. if it's a VAL containing the number 2, this should actually eval to -2
Sixth bit: the following VAR is boolean-negated - ie. it should eval to "false" if the following value evals to "true" and vice versa
Seventh bit: the following VAR should have the GML "~" operator applied
*/


class CodeRunner {
	private:
		AssetManager* _assetManager;
		InstanceList* _instances;

		// Internal code object
		struct CRCodeObject {
			char* code;
			unsigned int codeLength;
			bool question;
			unsigned char* compiled;
		};
		std::vector<CRCodeObject> _codeObjects;

		// States a GMLType can be in
		enum GMLTypeState {
			GML_TYPE_UNINIT,
			GML_TYPE_DOUBLE,
			GML_TYPE_STRING
		};

		// The universal data type in GML
		struct GMLType {
			GMLTypeState state;
			double dVal;
			char* sVal;
		};

		// Constants that can be referred to by compiled code
		std::vector<GMLType> _constants;

		// Field name index during compilation
		std::vector<char*> _fields;

		// Lists of internal names for compiling and running against
		std::vector<const char*> _internalFuncNames;
		std::vector<const char*> _gameValueNames;
		std::vector<const char*> _instanceVarNames;
		std::map<const char*, unsigned char> _gmlConsts;
		std::map<const char*, CROperator> _operators;
		std::map<const char*, CROperator> _ANOperators; //AlphaNumeric

		// Internal register used for loops and such
		std::stack<int> _stack;

		// Compile some code and return the position of the compiled code in outHandle.
		// Returns true on success, false on failure to compile (ie. program should exit.)
		bool _CompileCode(const char* str, unsigned char** outHandle, bool session = false);

		// Compile an expression and return the position of the compiled expression in outHandle.
		// Returns true on success, false on failure to compile (ie. program should exit.)
		bool _CompileExpression(const char* str, unsigned char** outHandle, bool session = false, unsigned int* outCharsUsed = NULL, unsigned int* outSize = NULL);


		// Helper functions for compiling
		unsigned int _RegConstantDouble(double d);
		unsigned int _RegConstantString(const char* c, unsigned int len);
		unsigned int _RegField(const char* c, unsigned int len);
		std::string substituteConstants(std::string code);
		bool _makeVal(const char* exp, unsigned int len, unsigned char* out);
		CRVarType _getVarType(std::string name, unsigned int* index = NULL);
		bool _getExpression(std::string input, unsigned int* pos, unsigned char* outVal);
		bool _isAsset(const char* name, unsigned int* index);


		// Helper functions for running
		bool _parseVal(const unsigned char* val, Instance* self, Instance* other, GMLType* out);
		bool _setGameValue(CRGameVar index, const unsigned char* arrayIndexVal, CRSetMethod method, GMLType value);
		bool _setInstanceVar(CRGameVar index, const unsigned char* arrayIndexVal, CRSetMethod method, GMLType value);
		bool _evalExpression(CodeObject obj, Instance* self, Instance* other, GMLType* out);
		bool _isTrue(const GMLType* value);

	public:
		CodeRunner(AssetManager* assets, InstanceList* instances);
		~CodeRunner();

		// For populating the constant vectors. This should be called before anything else.
		bool Init();

		// Register a code block to be compiled. Returns a unique reference to that code action to be later Compile()d and Run().
		CodeObject Register(char* code, unsigned int length);

		// Register a code expression to be compiled. Returns a unique reference to that code action to be later Compile()d and Query()d.
		CodeObject RegisterQuestion(char* code, unsigned int length);

		// Compile a code object that has been returned by Register(). Returns true on success, false on error (ie. game should close.)
		// Be sure to call this only after the AssetManager is fully loaded.
		bool Compile(CodeObject object);

		// Run a compiled code object. Returns true on success, false on error (ie. the game should close.)
		// Most be passed the instance ID of the "self" and "other" instances in this context. ("other" may be NULL.)
		bool Run(CodeObject code, Instance* self, Instance* other);

		// Run a compiled GML question (boolean expression). Returns true on success, false on error (ie. the game should close.)
		// The output value is stored in the supplied pointer.
		bool Query(CodeObject code, Instance* self, Instance* other, bool* response);
};

#endif