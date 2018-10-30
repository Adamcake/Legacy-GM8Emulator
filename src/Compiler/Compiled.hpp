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

  protected:
    virtual bool _evaluate(GMLType* output) = 0;

  public:
    bool Evaluate(GMLType* output);
    inline CROperator GetOperator() { return _operator; }
    inline void SetOperator(CROperator op) { _operator = op; }
    inline void SetUnaries(const std::vector<CRUnaryOperator>&& v) { _unary = v; }
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

class CRActionBindVars : public CRAction {
  private:
    std::vector<unsigned int> _fields;

  public:
    virtual bool Run() override;
    CRActionBindVars(std::vector<unsigned int>& fields) : _fields(fields) {}
};

class CRActionAssignmentField : public CRAction {
  private:
    unsigned int _field;
    CRSetMethod _method;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;

  public:
    CRActionAssignmentField(unsigned int field, CRSetMethod method, CRExpression exp) :
        _field(field), _method(method), _expression(exp), _hasDeref(false) {}
    CRActionAssignmentField(unsigned int field, CRSetMethod method, CRExpression deref, CRExpression exp) :
        _field(field), _method(method), _deref(deref), _expression(exp), _hasDeref(true) {}
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
    CRActionAssignmentArray(unsigned int field, CRSetMethod method, std::vector<CRExpression>& dimensions, CRExpression exp) :
        _field(field), _method(method), _expression(exp), _dimensions(dimensions), _hasDeref(false) {}
    CRActionAssignmentArray(unsigned int field, CRSetMethod method, std::vector<CRExpression>& dimensions, CRExpression deref, CRExpression exp) :
        _field(field), _method(method), _dimensions(dimensions), _deref(deref), _expression(exp), _hasDeref(true) {}
    virtual bool Run() override;
};

class CRActionAssignmentInstanceVar : public CRAction {
  private:
    CRInstanceVar _var;
    CRSetMethod _method;
    std::vector<CRExpression> _dimensions;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;

  public:
    CRActionAssignmentInstanceVar(CRInstanceVar var, CRSetMethod method, std::vector<CRExpression>& dimensions, CRExpression exp) :
        _var(var), _method(method), _expression(exp), _dimensions(dimensions), _hasDeref(false) {}
    CRActionAssignmentInstanceVar(CRInstanceVar var, CRSetMethod method, std::vector<CRExpression>& dimensions, CRExpression deref, CRExpression exp) :
        _var(var), _method(method), _dimensions(dimensions), _deref(deref), _expression(exp), _hasDeref(true) {}
    virtual bool Run() override;
};

class CRActionAssignmentGameVar : public CRAction {
  private:
    CRGameVar _var;
    CRSetMethod _method;
    std::vector<CRExpression> _dimensions;
    CRExpression _expression;

  public:
    CRActionAssignmentGameVar(CRGameVar var, CRSetMethod method, std::vector<CRExpression>& dimensions, CRExpression expression) :
        _var(var), _method(method), _dimensions(dimensions), _expression(expression) {}
    virtual bool Run() override;
};

class CRActionBlock : public CRAction {
  private:
    CRActionList _list;

  public:
    CRActionBlock(CRActionList list) : _list(list) {}
    virtual bool Run() override;
};

class CRActionRunFunction : public CRAction {
  private:
    CRInternalFunction _function;
    std::vector<CRExpression> _args;

  public:
    CRActionRunFunction(CRInternalFunction func, std::vector<CRExpression>& args) : _function(func), _args(args) {}
    virtual bool Run() override;
};

class CRActionRunScript : public CRAction {
  private:
    unsigned int _scriptID;
    std::vector<CRExpression> _args;

  public:
    CRActionRunScript(unsigned int id, std::vector<CRExpression>& args) : _scriptID(id), _args(args) {}
    virtual bool Run() override;
};

class CRActionIfElse : public CRAction {
  private:
    CRAction* _if;
    CRAction* _else;
    CRExpression _expression;

  public:
    CRActionIfElse(CRAction* i, CRAction* e, CRExpression exp) : _if(i), _else(e), _expression(exp) {}
    virtual bool Run() override;
};

class CRActionWith : public CRAction {
  private:
    CRExpression _expression;
    CRAction* _code;

  public:
    CRActionWith(CRExpression exp, CRAction* code) : _expression(exp), _code(code) {}
    virtual bool Run() override;
};

class CRActionSwitch : public CRAction {
  private:
    CRExpression _expression;
    CRActionList _actions;
    std::map<int, unsigned int> _offsets;

  public:
    CRActionSwitch(CRExpression exp, CRActionList actions, std::map<int, unsigned int>& offsets) : _expression(exp), _actions(actions), _offsets(offsets) {}
    virtual bool Run() override;
};

class CRActionBreak : public CRAction {
  public:
    CRActionBreak() {}
    virtual bool Run() override;
};

class CRActionContinue : public CRAction {
  public:
    CRActionContinue() {}
    virtual bool Run() override;
};

class CRActionExit : public CRAction {
  public:
    CRActionExit() {}
    virtual bool Run() override;
};

class CRActionReturn : public CRAction {
  private:
    CRExpression _expression;

  public:
    CRActionReturn(CRExpression exp) : _expression(exp) {}
    virtual bool Run() override;
};

// Expression value types

class CRExpLiteral : public CRExpressionValue {
  private:
    GMLType _value;

  public:
    CRExpLiteral(const double& d) {
        _value.state = GMLTypeState::Double;
        _value.dVal = d;
    }
    CRExpLiteral(const std::string& s) {
        _value.state = GMLTypeState::String;
        _value.sVal = s;
    }
    bool _evaluate(GMLType* output) override;
};

class CRExpFunction : public CRExpressionValue {
  private:
    CRInternalFunction _function;
    std::vector<CRExpression> _args;

  public:
    CRExpFunction(CRInternalFunction func, std::vector<CRExpression>& args) : _function(func), _args(args) {}
    bool _evaluate(GMLType* output) override;
};

class CRExpScript : public CRExpressionValue {
  private:
    unsigned int _script;
    std::vector<CRExpression> _args;

  public:
    CRExpScript(unsigned int id, std::vector<CRExpression>& args) : _script(id), _args(args) {}
    bool _evaluate(GMLType* output) override;
};

class CRExpNestedExpression : public CRExpressionValue {
  private:
    CRExpression _expression;

  public:
    CRExpNestedExpression(CRExpression exp) : _expression(exp) {}
    bool _evaluate(GMLType* output) override;
};

class CRExpField : public CRExpressionValue {
  private:
    unsigned int _fieldNumber;
    CRExpression _deref;
    bool _hasDeref;

  public:
    CRExpField(unsigned int field) : _fieldNumber(field), _hasDeref(false) {}
    CRExpField(unsigned int field, CRExpression deref) : _fieldNumber(field), _deref(deref), _hasDeref(true) {}
    bool _evaluate(GMLType* output) override;
};

class CRExpArray : public CRExpressionValue {
  private:
    unsigned int _fieldNumber;
    std::vector<CRExpression> _dimensions;
    CRExpression _deref;
    bool _hasDeref;

  public:
    CRExpArray(unsigned int field, std::vector<CRExpression>& dimensions) : _fieldNumber(field), _dimensions(dimensions), _hasDeref(false) {}
    CRExpArray(unsigned int field, std::vector<CRExpression>& dimensions, CRExpression deref) : _fieldNumber(field), _dimensions(dimensions), _deref(deref), _hasDeref(true) {}
    bool _evaluate(GMLType* output) override;
};

class CRExpInstanceVar : public CRExpressionValue {
  private:
    CRInstanceVar _var;
    std::vector<CRExpression> _dimensions;
    CRExpression _deref;
    bool _hasDeref;

  public:
    CRExpInstanceVar(CRInstanceVar var, std::vector<CRExpression>& dimensions) : _var(var), _dimensions(dimensions), _hasDeref(false) {}
    CRExpInstanceVar(CRInstanceVar var, std::vector<CRExpression>& dimensions, CRExpression deref) : _var(var), _dimensions(dimensions), _deref(deref), _hasDeref(true) {}
    bool _evaluate(GMLType* output) override;
};

class CRExpGameVar : public CRExpressionValue {
  private:
    CRGameVar _var;
    std::vector<CRExpression> _dimensions;

  public:
    CRExpGameVar(CRGameVar var, std::vector<CRExpression>& dimensions) : _var(var), _dimensions(dimensions) {}
    bool _evaluate(GMLType* output) override;
};

#endif