#ifndef _A_CREXPRESSION_HPP_
#define _A_CREXPRESSION_HPP_
#include <vector>
#include "CREnums.hpp"

// Data structure for a part-compiled expression. From this, we can sort out things like operator precedence much more easily.
// This is a linked list implementation. An element consists of a VAR in compiled bytes, the list of modifiers for it,
// the following operator, and a pointer to the next element. (pointer is null/undefined if operator is STOP)

struct CRExpressionElement {
	std::vector<unsigned char> var;
	std::vector<CROperator> mods;
	CROperator op;
	CRExpressionElement* next;
};

#endif