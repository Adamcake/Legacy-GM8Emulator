#ifndef _A_COMPILED_HPP_
#define _A_COMPILED_HPP_
#include <pch.h>
#include "CREnums.hpp"
#include "CRGMLType.hpp"

// Abstract super-class for compiled actions
class CRAction {
  public:
    virtual bool Run() = 0;
};

// Abstract super-class for compiled expression values
class CRExpressionValue {
  private:
    std::vector<CRUnaryOperator> _unary;
    CROperator _operator;

  public:
    virtual bool Evaluate(GMLType* output) = 0;
    inline CROperator GetOperator() { return _operator; }
};

// List of actions
class CRActionList {
  private:
    std::vector<CRAction*> _actions;

  public:
    inline void Append(CRAction* a) { _actions.push_back(a); }
    bool Run(unsigned int start = 0);
};

// List of expression values
class CRExpression {
  private:
    std::vector<CRExpressionValue*> _values;

  public:
    inline void Append(CRExpressionValue* a) { _values.push_back(a); }
    bool Evaluate(GMLType* output);
};


// Actual action types

class CRActionAssignmentField : public CRAction {
  private:
    unsigned int _field;
    CRSetMethod _method;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;

  public:
    virtual bool Run() override;
};

class CRActionAssignmentArray : public CRAction {
  private:
    unsigned int _field;
    CRSetMethod _method;
    std::vector<CRExpression> _dimensions;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;

  public:
    virtual bool Run() override;
};

class CRActionAssignmentInstanceVar : public CRAction {
  private:
    CRInstanceVar _var;
    CRSetMethod _method;
    std::vector<CRExpression> _dimensions;
    bool _hasArray;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;

  public:
    virtual bool Run() override;
};

class CRActionAssignmentGameVar : public CRAction {
  private:
    CRGameVar _field;
    CRSetMethod _method;
    std::vector<CRExpression> _dimensions;
    bool _hasArray;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;

  public:
    virtual bool Run() override;
};

class CRActionBlock : public CRAction {
  private:
    CRActionList _list;

  public:
    virtual bool Run() override;
};

class CRActionRunFunction : public CRAction {
  private:
    CRInternalFunction _function;
    std::vector<CRExpression> _args;

  public:
    virtual bool Run() override;
};

class CRActionRunScript : public CRAction {
  private:
    unsigned int _scriptID;
    std::vector<CRExpression> _args;

  public:
    virtual bool Run() override;
};

class CRActionIfElse : public CRAction {
  private:
    CRAction* _if;
    CRAction* _else;
    CRExpression _expression;

  public:
    virtual bool Run() override;
};

class CRActionWith : public CRAction {
  private:
    CRExpression _expression;
    CRAction* _code;

  public:
    virtual bool Run() override;
};

class CRActionSwitch : public CRAction {
  private:
    CRActionList _actions;
    std::map<int, unsigned int> _offsets;

  public:
    virtual bool Run() override;
};

// Expression value types

class CRExpLiteral : public CRExpressionValue {
  private:
    GMLType _value;

  public:
    bool Evaluate(GMLType* output) override;
};

class CRExpFunction : public CRExpressionValue {
  private:
    CRInternalFunction _function;
    std::vector<CRExpression> _args;

  public:
    bool Evaluate(GMLType* output) override;
};

class CRExpScript : public CRExpressionValue {
  private:
    unsigned int _script;
    std::vector<CRExpression> _args;

  public:
    bool Evaluate(GMLType* output) override;
};

class CRExpNestedExpression : public CRExpressionValue {
  private:
    CRExpression _expression;

  public:
    bool Evaluate(GMLType* output) override;
};

class CRExpField : public CRExpressionValue {
  private:
    unsigned int _fieldNumber;
    CRExpression _deref;
    bool _hasDeref;

  public:
    bool Evaluate(GMLType* output) override;
};

class CRExpArray : public CRExpressionValue {
  private:
    unsigned int _arrayNumber;
    std::vector<CRExpression> _dimensions;
    CRExpression _deref;
    bool _hasDeref;

  public:
    bool Evaluate(GMLType* output) override;
};

class CRExpInstanceVar : public CRExpressionValue {
  private:
    CRInstanceVar _var;
    std::vector<CRExpression> _dimensions;
    bool _hasArray;
    CRExpression _deref;
    bool _hasDeref;

  public:
    bool Evaluate(GMLType* output) override;
};

class CRExpGameVar : public CRExpressionValue {
  private:
    CRGameVar _var;
    std::vector<CRExpression> _dimensions;
    bool _hasArray;
    CRExpression _deref;
    bool _hasDeref;

  public:
    bool Evaluate(GMLType* output) override;
};

#endif