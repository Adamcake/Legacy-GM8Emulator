#ifndef _A_CODERUNNER_HPP_
#define _A_CODERUNNER_HPP_
class AssetManager;
class InstanceList;
struct Instance;
#include <vector>

typedef unsigned int CodeObject;

class CodeRunner {
	private:
		AssetManager* _assetManager;
		InstanceList* _instances;

		struct CRCodeObject {
			char* code;
			unsigned int codeLength;
			bool question;
		};
		std::vector<CRCodeObject> _codeObjects;

	public:
		CodeRunner(AssetManager* assets, InstanceList* instances);
		~CodeRunner();

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