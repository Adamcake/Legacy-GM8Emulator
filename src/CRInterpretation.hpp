#ifndef _A_CRINTERPRETATION_HPP_
#define _A_CRINTERPRETATION_HPP_
#include <vector>
#include <map>
struct CRStatement;

// Compiles a list of statements produced by _InterpretCode.
void _CompileStatements(const std::vector<CRStatement*>* const statements, std::vector<unsigned char>* output);

// A Statement is a single line of code, post-interpretation.
// This is the abstract superclass.
struct CRStatement {
	virtual void write(std::vector<unsigned char>* output) const = 0;
	virtual ~CRStatement() {}
};

// All implementing classes are prefixed with CRS (CodeRunner Statement)
// All implementing classes must override write() with own implementation.

struct CRSExit : public CRStatement {
	void write(std::vector<unsigned char>* output) const override;
};

struct CRSBindVars : public CRStatement {
	void write(std::vector<unsigned char>* output) const override;
	std::vector<unsigned int> _vars;
};

struct CRSWhile : public CRStatement {
	unsigned char _test[3];
	std::vector<CRStatement*> _code;
	void write(std::vector<unsigned char>* output) const override;
	~CRSWhile() override { for (CRStatement* s : _code) delete s; }
};

struct CRSDoUntil : public CRStatement {
	std::vector<CRStatement*> _code;
	unsigned char _test[3];
	void write(std::vector<unsigned char>* output) const override;
	~CRSDoUntil() override { for (CRStatement* s : _code) delete s; }
};

struct CRSFor : public CRStatement {
	std::vector<CRStatement*> _init;
	std::vector<CRStatement*> _final;
	unsigned char _test[3];
	std::vector<CRStatement*> _code;
	void write(std::vector<unsigned char>* output) const override;
	~CRSFor() override { for (CRStatement* s : _code) delete s; for (CRStatement* s : _init) delete s; for (CRStatement* s : _final) delete s; }
};

struct CRSRepeat : public CRStatement {
	unsigned char _count[3];
	std::vector<CRStatement*> _code;
	void write(std::vector<unsigned char>* output) const override {} // TODO
	~CRSRepeat() override { for (CRStatement* s : _code) delete s; }
};

struct CRSIf : public CRStatement {
	unsigned char _test[3];
	std::vector<CRStatement*> _code;
	std::vector<CRStatement*> _elseCode; // If there's no "else", this is empty
	void write(std::vector<unsigned char>* output) const override;
	~CRSIf() override { for (CRStatement* s : _code) delete s; for (CRStatement* s : _elseCode) delete s; }
};

struct CRSwitchCase {
	bool _default = false;
	unsigned char _val[3];
	unsigned int _offset;
};
struct CRSSwitch : public CRStatement {
	unsigned char _val[3];
	std::vector<CRStatement*> _code;
	std::vector<CRSwitchCase> _cases;
	void write(std::vector<unsigned char>* output) const override;
	~CRSSwitch() override { for (CRStatement* s : _code) delete s; }
};

struct CRSWith : public CRStatement {
	unsigned char _id[3];
	std::vector<CRStatement*> _code;
	void write(std::vector<unsigned char>* output) const override;
};

struct CRSBreak : public CRStatement {
	void write(std::vector<unsigned char>* output) const override {} // TODO
};

struct CRSContinue : public CRStatement {
	void write(std::vector<unsigned char>* output) const override {} // TODO
};

struct CRSReturn : public CRStatement {
	unsigned char _val[3];
	void write(std::vector<unsigned char>* output) const override;
};

struct CRSUserScript : public CRStatement {
	unsigned int _id;
	unsigned int _argCount;
	std::vector<unsigned char> _args;
	void write(std::vector<unsigned char>* output) const override;
};

struct CRSFunction : public CRStatement {
	unsigned int _id;
	unsigned int _argCount;
	std::vector<unsigned char> _args;
	void write(std::vector<unsigned char>* output) const override;
};



struct CRSPreCompiled : public CRStatement { // For bytecode that is already compiled
	std::vector<unsigned char> _code;
	void write(std::vector<unsigned char>* output) const override;
};

#endif