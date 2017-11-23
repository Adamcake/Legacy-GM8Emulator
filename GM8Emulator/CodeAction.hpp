#ifndef _A_CODEACTION_HPP_
#define _A_CODEACTION_HPP_

typedef unsigned int CodeObject;
class CodeRunner;
struct Instance;

// This class is for specific instances of drag-n-drop actions. The type of action is indicated by its actionID.
// Note that actions are unique, ie. there can never be two references to the same action from different places in the game logic.
class CodeAction {
	private:
		unsigned int actionID;
		CodeObject codeObj;
		bool question;
		unsigned int param; // Currently only used for repeat blocks because the game object needs to know how many times to repeat.

	public:
		CodeAction();
		~CodeAction();

		// Read code action from EXE data stream, prepares and registers a GML block with the code runner.
		// Returns true on success, false on error (ie. game should close.)
		bool Read(const unsigned char* stream, unsigned int* pos, CodeRunner* runner);

		// Compile self into GML and store code reference. Returns true on success, false on error (ie. game should close.)
		// Only do this after the asset list is fully loaded.
		bool Compile(CodeRunner* runner);

		// Run this action. Returns true on success, false on error (ie. game should close.)
		// If this is a question, supply a pointer to a boolean where the response will be output.
		bool Run(CodeRunner* runner, Instance* self, Instance* other, bool* response = nullptr);

		// Tells whether this type of action is a question - if so, the action (or action block) immediately after it should be dependent on this returning "true".
		inline bool IsQuestion() { return question; }
};

#endif