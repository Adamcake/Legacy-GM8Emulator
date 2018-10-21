#ifndef _A_CODERUNNER_HPP_
#define _A_CODERUNNER_HPP_

struct CRStatement;
struct GlobalValues;
struct Instance;
struct CRExpressionElement;
struct GMLType;
enum struct GMLTypeState;
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

class CodeRunner {
	private:
		// Internal code object
		struct CRCodeObject {
			std::string code;
			unsigned int codeLength;
			bool question;
			unsigned char* compiled;
		};
		std::vector<CRCodeObject> _codeObjects;

		// Runtime context
        /*
		struct CRContext {
			Instance* self;
			Instance* other;
			int eventId;
			int eventNumber;
			unsigned int objId;
			unsigned int argc;
			GMLType* argv;
			InstanceList::Iterator iterator;
			//std::map<unsigned int, GMLType> locals;
			CRContext(Instance* s, Instance* o, int e, int se, unsigned int oid, unsigned int ac = 0, GMLType* av = NULL) : self(s), other(o), eventId(e), eventNumber(se), objId(oid), argc(ac), argv(av), iterator(0) {}
			CRContext(Instance* o, unsigned int id, unsigned int ac = 0, GMLType* av = NULL) : other(o), iterator(id), argc(ac), argv(av) { self = iterator.Next(); }
		};
		std::stack<CRContext> _contexts;
        */

		// Structure for user file manipulation
		//std::fstream _userFiles[32];

		// Field name index during compilation
		//std::vector<char*> _fieldNames;

		// Lists of internal names for compiling and running against
		std::vector<const char*> _internalFuncNames;
		std::vector<const char*> _gameValueNames;
		std::vector<const char*> _instanceVarNames;
		std::map<const char*, int> _gmlConsts;
		std::vector<bool(*)(unsigned int,GMLType*,GMLType*)> _gmlFuncs;

		// Iterator stack for runtime
		//std::stack<InstanceList::Iterator> _iterators;

		// The next instance ID to assign when instance_create is called
		//unsigned int _nextInstanceID;

		// Room order
		//unsigned int** _roomOrder;
		//unsigned int _roomOrderCount;


	public:
		CodeRunner(GlobalValues* globals);
		~CodeRunner();

		// For populating the constant lists. This should be called before anything else.
		bool Init();

		// Set a specific room order. Usually done after loading the room order from the exe.
		void SetRoomOrder(unsigned int** order, unsigned int count);

		// Set the next instance ID to assign after all the static instances are loaded
		//inline void SetNextInstanceID(unsigned int i) { _nextInstanceID = i; }

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