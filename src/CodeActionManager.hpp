#ifndef _A_CODEACTION_HPP_
#define _A_CODEACTION_HPP_
#include <vector>

typedef unsigned int CodeAction;
typedef unsigned int CodeObject;
class CodeRunner;
struct Instance;

class CodeActionManager {
	struct CACodeAction {
		unsigned int actionID;
		CodeObject codeObj;
		bool question;
		unsigned int param; // Currently only used for repeat blocks because the game object needs to know how many times to repeat.
	};
	std::vector<CACodeAction> _actions;
	CodeRunner* _runner;

	public:
		// Read code action from EXE data stream, prepares and registers a GML block with the code runner.
		// Returns true on success, false on error (ie. game should close.) Outputs CodeAction reference in the "out" param.
		bool Read(const unsigned char* stream, unsigned int* pos, CodeAction* out);

		// Compile an action into GML and store code reference. Returns true on success, false on error (ie. game should close.)
		// Only do this after the asset list is fully loaded.
		bool Compile(CodeAction action);

		// Run a list of actions. Returns true on success, false on error (ie. game should close.)
		bool Run(CodeAction* actions, unsigned int count, Instance* self, Instance* other);

		// Must be done before
		inline void SetRunner(CodeRunner* runner) { _runner = runner; }
};

#endif