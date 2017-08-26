#ifndef _A_CODEACTION_HPP_
#define _A_CODEACTION_HPP_

// This class is for specific instances of drag-n-drop actions. The type of action is indicated by its actionID.
// Note that actions are unique, ie. there can never be two references to the same action from different places in the game logic.
class CodeAction {
	private:
		unsigned int actionID;

		unsigned int argCount;
		char** args;

		int appliesToObject;
		bool question;
		bool relative;
		bool not;

	public:
		CodeAction();
		~CodeAction();

		bool read(const unsigned char* stream, unsigned int* pos);
};

#endif