#ifndef _A_CODERUNNER_HPP_
#define _A_CODERUNNER_HPP_

class CodeActionManager;
struct CRStatement;
struct GlobalValues;
struct Instance;
struct CRExpressionElement;
typedef unsigned int CodeObject;
typedef unsigned int InstanceID;
enum CRGameVar;
enum CRInstanceVar;
enum CRVarType;
enum CROperator;
enum CRSetMethod;
#include <pch.h>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <string>
#include "InstanceList.hpp"

#define PI 3.141592654 // Actual value of PI used by the official runner. Please don't make it more accurate.

// if true, calling unimplemented GML functions will cause an error
// GET THAT 'static' OUT OF MY PROJECT!!
constexpr bool CRErrorOnUnimplemented = false;

/*
The runner compiles GML into an instruction format that is based on the format of x86 CPU instructions, except massively cut down. This is (vaguely) documented here, for now.

VAL:
Wherever "VAL" is used below, it refers to a 3-byte structure where the first 2 bits indicates the nature of the value, and the following 22 bits is the value itself.
The bytes for the latter part are big-endian, ie. the 6 bits in the first byte are only used in the case of a number too large for 2 bytes.
Byte 1 can have the following values:
- 0: top int stack variable (other bytes discarded)
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

05: Set array (eg self.abc[1] or self.abc[1,2])
Followed by 12 bytes. The first 2 indicate the array number, the following two sets of 3 are VALs indicating the 2d array indexes,
the following byte indicates the SET METHOD and the final 3 are the VAL for what to set it to.
This will set the indicated array index of whatever is stored in the dereferencing buffer (default is self.)
NOTE: GML supports 1d and 2d arrays but actually all arrays are 2d. For example, a[3] and a[0,3] access the same value.

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

0B: Test a value (next JMP instruction is skipped if the VAL evaluates to false)
Followed by 3 bytes indicating the VAL to test. If the next op isn't a jump, the test result is discarded.

0C: Test a value for falseness (next JMP instruction is skipped if the VAL evaluates to true)
Followed by 3 bytes indicating the VAL to test. If the next op isn't a jump, the test result is discarded.

0D: Test two values for equality (next JMP instruction is skipped if the VALs do not match)
Followed by 6 bytes indicating the two VALs to test. If the next op isn't a jump, the test result is discarded.

0E: Change context
Followed by 6 bytes. The first 3 indicate a VAL to dereference and use as "self". The last 3 indicate how far to JMP ahead (excluding this instruction) if no instances are found.
"other" will be set to the previous "self". The current state before changing will be pushed onto a session stack.

0F: Revert context
Followed by 0 bytes. Goes back to the start of the context and changes the "self" to be the next one iterated. If there are no more instances,
then the context will be popped from the stack and running will continue from here.

10: Jump ahead short
Followed by 1 byte. The byte indicates how many instructions to jump ahead AFTER that byte.

11: Jump ahead long
Followed by 3 bytes. The bytes indicate how many instructions to jump ahead AFTER those bytes.

12: Jump back short
Followed by 1 byte. The byte indicates how many instructions to jump back AFTER that byte (ie. the distance must include the 2 bytes used by this instruction.)

13: Jump back long
Followed by 3 bytes. The bytes indicate how many instructions to jump back AFTER those bytes (ie. the distance must include the 4 bytes used by this instruction.)

14: Set top integer stack value
Followed by 3 bytes indicating a VAL. Will be rounded.

15: Set top var stack value
Followed by 3 bytes indicating a VAL.

16: Push onto integer stack
Followed by 0 bytes. The value pushed is 0.

17: Pop from integer stack
Followed by 0 bytes.

18: Push onto var stack
Followed by 0 bytes. The var pushed will be a real with value 0.0.

19: Pop from var stack
Followed by 0 bytes

1A: Return
Followed by 3 bytes indicating a VAL. (Writes to the return buffer and exits.)


========

EXPRESSIONS:
An expression is some GML that gets compiled and can later be evaluated, returning a GMLType. It's formatted as:
VAR [MODS] [OPERATOR VAR [MODS] [OPERATOR VAR [MODS] [OPERATOR VAR [MODS]...]] OPERATOR
For example, in the GML line "player.x = mouse_x - 1", there is an expression: "mouse_x - 1". mouse_x is a VAR, 1 is a VAR, and - is an OP. 
The final OPERATOR must be a "STOP" to terminate the expression. OPERATOR, VAR and MODS defined below:


"OPERATOR" refers to an operator which acts on two VARs (the LHS and RHS). These can have the following values:
0x00: STOP (end of expression)
0x01: +
0x02: -
0x03: *
0x04: /
0x05: mod
0x06: div
0x07: == (can also be "=" in gml expression)
0x08: !=
0x09: <
0x0A: <=
0x0B: >
0x0C: >=
0x0D: |
0x0E: || (can also be "or" in gml)
0x0F: &
0x10: && (can also be "and" in gml)
0x11: ^
0x12: ^^ (can also be "xor" in gml)
0x13: <<
0x14: >>
0x15: .


"VAR" here refers to a variable in an expression. Like with instructions, the first byte indicates what follows it.

VALUE TABLE:

00: Unused

01: VAL
Followed by 3 bytes which are interpreted as a VAL.

02: Get game value (room_speed, health, etc)
Followed by 4 bytes, the first indicates which game value to get and the following 3 are a VAL for the array index if applicable - otherwise discarded.

03: Get field (eg self.abc)
Followed by 2 bytes which indicate the field number.
This will read from the instance in the expression dereference buffer (default is self.)

04: Get array (eg self.abc[2], self.abc[1,2])
Followed by 8 bytes. The first 2 indicate the array number, the final two sets of 3 are VALs indicating the 2D array indexes.
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


MODS (AKA. Unary Operators):
After the VAR may be some modifiers which should be applied to the calculated VAR left-to-right.
The values listed below must not overlap with the available OPERATOR values, as this would create ambiguous bytes.
0x16: NOT (!)
0x17: NEGATIVE (-)
0x18: TILDE (~)
*/


class CodeRunner {
	private:
		InstanceList* _instances;
		GlobalValues* _globalValues;
		CodeActionManager* _codeActions;

		// Internal code object
		struct CRCodeObject {
			char* code;
			unsigned int codeLength;
			bool question;
			unsigned char* compiled;
		};
		std::vector<CRCodeObject> _codeObjects;

		// States a GMLType can be in
		enum struct GMLTypeState {
			Double,
			String
		};

		// The universal data type in GML
		struct GMLType {
			GMLTypeState state = GMLTypeState::Double;
			double dVal = 0.0;
			std::string sVal;
		};

		// Runtime context
		struct CRContext {
			Instance* self;
			Instance* other;
			int eventId;
			int eventNumber;
			unsigned int objId;
			unsigned int startpos;
			unsigned int argc;
			GMLType* argv;
			InstanceList::Iterator iterator;
			std::map<unsigned int, GMLType> locals;
			CRContext(Instance* s, Instance* o) : self(s), other(o), startpos(0), iterator(NULL, 0) {}
			CRContext(Instance* s, Instance* o, int e, int se, unsigned int oid, unsigned int ac = 0, GMLType* av = NULL) : self(s), other(o), eventId(e), eventNumber(se), startpos(0), objId(oid), argc(ac), argv(av), iterator(NULL, 0) {}
			CRContext(Instance* o, unsigned int start, InstanceList* list) : other(o), startpos(start), iterator(list) { self = iterator.Next(); }
			CRContext(Instance* o, unsigned int start, InstanceList* list, unsigned int id) : other(o), startpos(start), iterator(list, id) { self = iterator.Next(); }
		};
		std::stack<CRContext> _contexts;

		// Structure for user file manipulation
		std::fstream _userFiles[32];

		// Constants that can be referred to by compiled code
		std::vector<GMLType> _constants;

		// Field name index during compilation
		std::vector<char*> _fieldNames;

		// Lists of internal names for compiling and running against
		std::vector<const char*> _internalFuncNames;
		std::vector<const char*> _gameValueNames;
		std::vector<const char*> _instanceVarNames;
		std::map<const char*, int> _gmlConsts;
		std::map<const char*, CROperator> _operators;
		std::map<const char*, CROperator> _ANOperators; //AlphaNumeric
		std::vector<bool(CodeRunner::*)(unsigned int,GMLType*,GMLType*)> _gmlFuncs;

		// Internal registers used for loops and such
		std::stack<int> _stack;
		std::stack<GMLType> _varstack;

		// Field map
		std::map<InstanceID, std::map<unsigned int, GMLType>> _fields;

		// Array map (if you're maintaining this: god help you)
		std::map<InstanceID, std::map<unsigned int, std::map<int, std::map<int, GMLType>>>> _arrays;

		// Iterator stack for runtime
		std::stack<InstanceList::Iterator> _iterators;

		// The next instance ID to assign when instance_create is called
		unsigned int _nextInstanceID;

		// Room order
		unsigned int** _roomOrder;
		unsigned int _roomOrderCount;

		// Interpret some code and return the position of the compiled code in outHandle.
		// Returns true on success, false on failure to compile (ie. program should exit.)
		bool _InterpretCode(const char* str, std::vector<CRStatement*>* output, bool session = false);

		// Compile an expression and return the position of the compiled expression in outHandle.
		// Returns true on success, false on failure to compile (ie. program should exit.)
		bool _CompileExpression(const char* str, unsigned char** outHandle, bool session = false, unsigned int* outCharsUsed = NULL, unsigned int* outSize = NULL);

		// Helper functions for compiling
		bool _InterpretLine(std::string str, unsigned int* pos, std::vector<CRStatement*>* output);
		unsigned int _RegConstantDouble(double d);
		unsigned int _RegConstantString(const char* c, unsigned int len);
		unsigned int _RegField(const char* c, unsigned int len);
		std::string substituteConstants(std::string code);
		bool _makeVal(const char* exp, unsigned int len, unsigned char* out);
		CRVarType _getVarType(std::string name, unsigned int* index = NULL);
		bool _getExpression(std::string input, unsigned int* pos, unsigned char* outVal);
		bool _compileExpStruct(CRExpressionElement* exp, unsigned char** out, unsigned int* outSize);
		bool _isAsset(const char* name, unsigned int* index);

		// Helper functions for running
		int _round(double);
		bool _equal(double, double);
		bool _runCode(const unsigned char* code, GMLType* out);
		bool _parseVal(const unsigned char* val, GMLType* out);
		bool _setGameValue(CRGameVar index, const unsigned char* arrayIndexVal, CRSetMethod method, GMLType value);
		bool _getGameValue(CRGameVar index, const unsigned char* arrayIndexVal, GMLType* out);
		bool _setInstanceVar(Instance* instance, CRInstanceVar index, const unsigned char* arrayIndexVal, CRSetMethod method, GMLType value);
		bool _getInstanceVar(Instance* instance, CRInstanceVar index, const unsigned char* arrayIndexVal, GMLType* out);
		bool _evalExpression(unsigned char* code, GMLType* out);
		bool _readExpVal(unsigned char* code, unsigned int* pos, Instance* derefBuffer, GMLType* argStack, GMLType* out);
		bool _isTrue(const GMLType* value);
		bool _applySetMethod(GMLType* lhs, CRSetMethod method, const GMLType* const rhs);
		bool _assertArgs(unsigned int & argc, GMLType *argv, unsigned int arge, bool lenient, ...);

		// "draw" vars
		unsigned int _drawFont = -1;
		int _drawColour = 0;
		int _drawValign = 0;
		int _drawHalign = 0;
		double _drawAlpha = 1.0;

		// GML internal functions
		bool abs(unsigned int argc, GMLType* argv, GMLType* out);
		bool choose(unsigned int argc, GMLType* argv, GMLType* out);
		bool cos(unsigned int argc, GMLType* argv, GMLType* out);
		bool distance_to_object(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_rectangle(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_alpha(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_color(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_font(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_halign(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_valign(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_sprite(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_sprite_ext(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_text(unsigned int argc, GMLType* argv, GMLType* out);
		bool event_inherited(unsigned int argc, GMLType* argv, GMLType* out);
		bool event_perform(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_open(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_close(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_read_byte(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_write_byte(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_delete(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_exists(unsigned int argc, GMLType* argv, GMLType* out);
		bool floor(unsigned int argc, GMLType* argv, GMLType* out);
		bool game_end(unsigned int argc, GMLType* argv, GMLType* out);
		bool game_restart(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_create(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_destroy(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_exists(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_number(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_position(unsigned int argc, GMLType* argv, GMLType* out);
		bool irandom(unsigned int argc, GMLType* argv, GMLType* out);
		bool irandom_range(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check_direct(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check_pressed(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check_released(unsigned int argc, GMLType* argv, GMLType* out);
		bool make_color_hsv(unsigned int argc, GMLType* argv, GMLType* out);
		bool max(unsigned int argc, GMLType* argv, GMLType* out);
		bool min(unsigned int argc, GMLType* argv, GMLType* out);
		bool move_contact_solid(unsigned int argc, GMLType* argv, GMLType* out);
		bool move_wrap(unsigned int argc, GMLType* argv, GMLType* out);
		bool ord(unsigned int argc, GMLType* argv, GMLType* out);
		bool place_free(unsigned int argc, GMLType* argv, GMLType* out);
		bool place_meeting(unsigned int argc, GMLType* argv, GMLType* out);
		bool point_direction(unsigned int argc, GMLType* argv, GMLType* out);
		bool power(unsigned int argc, GMLType* argv, GMLType* out);
		bool random(unsigned int argc, GMLType* argv, GMLType* out);
		bool random_range(unsigned int argc, GMLType* argv, GMLType* out);
		bool random_get_seed(unsigned int argc, GMLType* argv, GMLType* out);
		bool random_set_seed(unsigned int argc, GMLType* argv, GMLType* out);
		bool room_goto(unsigned int argc, GMLType* argv, GMLType* out);
		bool room_goto_next(unsigned int argc, GMLType* argv, GMLType* out);
		bool room_goto_previous(unsigned int argc, GMLType* argv, GMLType* out);
		bool sign(unsigned int argc, GMLType* argv, GMLType* out);
		bool sin(unsigned int argc, GMLType* argv, GMLType* out);
		bool sqr(unsigned int argc, GMLType* argv, GMLType* out);
		bool sqrt(unsigned int argc, GMLType* argv, GMLType* out);
		bool string(unsigned int argc, GMLType* argv, GMLType* out);
		bool string_width(unsigned int argc, GMLType* argv, GMLType* out);
		bool string_height(unsigned int argc, GMLType* argv, GMLType* out);

		bool unimplemented(unsigned int argc, GMLType* argv, GMLType* out);

	public:
		CodeRunner(InstanceList* instances, GlobalValues* globals, CodeActionManager* codeActions);
		~CodeRunner();

		// For populating the constant lists. This should be called before anything else.
		bool Init();

		// Set a specific room order. Usually done after loading the room order from the exe.
		void SetRoomOrder(unsigned int** order, unsigned int count);

		// Set the next instance ID to assign after all the static instances are loaded
		inline void SetNextInstanceID(unsigned int i) { _nextInstanceID = i; }

		// Register a code block to be compiled. Returns a unique reference to that code action to be later Compile()d and Run().
		CodeObject Register(char* code, unsigned int length);

		// Register a code expression to be compiled. Returns a unique reference to that code action to be later Compile()d and Query()d.
		CodeObject RegisterQuestion(char* code, unsigned int length);

		// Compile a code object that has been returned by Register(). Returns true on success, false on error (ie. game should close.)
		// Be sure to call this only after the AssetManager is fully loaded.
		bool Compile(CodeObject object);

		// Run a compiled code object. Returns true on success, false on error (ie. the game should close.)
		// Most be passed the instance ID of the "self" and "other" instances in this context. (both may be NULL)
		// ev and sub indicate the event that's being run. For more info, check the "COMPILED OBJECT EVENTS" section of notes.txt
		bool Run(CodeObject code, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, unsigned int argc = 0, GMLType* argv = NULL);

		// Run a compiled GML question (boolean expression). Returns true on success, false on error (ie. the game should close.)
		// The output value is stored in the supplied pointer.
		bool Query(CodeObject code, Instance* self, Instance* other, bool* response);
};

#endif