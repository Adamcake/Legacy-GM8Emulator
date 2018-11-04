#ifndef _A_COMPILED_HPP_
#define _A_COMPILED_HPP_
#include <pch.h>
#include "CREnums.hpp"
#include "CRGMLType.hpp"

// Abstract super-class for compiled actions
class CRAction {
  public:
    virtual bool Run() = 0;
    virtual void Finalize() {}
    virtual ~CRAction() {}
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
    virtual void Finalize() {}
    virtual ~CRExpressionValue() {}

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
    virtual void Finalize();

    inline size_t Count() {return _actions.size();}
};

// List of expression values
class CRExpression {
  private:
    std::vector<CRExpressionValue*> _values;

  public:
    inline void Append(CRExpressionValue* a) { _values.push_back(a); }
    bool Evaluate(GMLType* output);
    virtual void Finalize();
};


// Actual action types

class CRActionBindVars : public CRAction {
  public:
    virtual bool Run() override;
    CRActionBindVars() {}
};

class CRActionAssignmentField : public CRAction {
  private:
    unsigned int _field;
    CRSetMethod _method;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;
    bool _isLocal;

  public:
    CRActionAssignmentField(unsigned int field, CRSetMethod method, CRExpression exp, bool isLocal) :
        _field(field), _method(method), _expression(exp), _hasDeref(false), _isLocal(isLocal) {}
    CRActionAssignmentField(unsigned int field, CRSetMethod method, CRExpression deref, CRExpression exp, bool isLocal) :
        _field(field), _method(method), _deref(deref), _expression(exp), _hasDeref(true), _isLocal(isLocal) {}
    virtual bool Run() override;
    virtual void Finalize() override {_deref.Finalize(); _expression.Finalize();}
};

class CRActionAssignmentArray : public CRAction {
  private:
    unsigned int _field;
    CRSetMethod _method;
    std::vector<CRExpression> _dimensions;
    CRExpression _deref;
    bool _hasDeref;
    CRExpression _expression;
    bool _isLocal;

  public:
    CRActionAssignmentArray(unsigned int field, CRSetMethod method, std::vector<CRExpression>& dimensions, CRExpression exp, bool isLocal) :
        _field(field), _method(method), _expression(exp), _dimensions(dimensions), _hasDeref(false), _isLocal(isLocal) {}
    CRActionAssignmentArray(unsigned int field, CRSetMethod method, std::vector<CRExpression>& dimensions, CRExpression deref, CRExpression exp, bool isLocal) :
        _field(field), _method(method), _dimensions(dimensions), _deref(deref), _expression(exp), _hasDeref(true), _isLocal(isLocal) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _deref.Finalize();
        _expression.Finalize();
        for(CRExpression& exp : _dimensions) {
            exp.Finalize();
        }
    }
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
    virtual void Finalize() override {
        _deref.Finalize();
        _expression.Finalize();
        for (CRExpression& exp : _dimensions) {
            exp.Finalize();
        }
    }
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
    virtual void Finalize() override {
        _expression.Finalize();
        for (CRExpression& exp : _dimensions) {
            exp.Finalize();
        }
    }
};

class CRActionBlock : public CRAction {
  private:
    CRActionList _list;

  public:
    CRActionBlock(CRActionList list) : _list(list) {}
    virtual bool Run() override;
    virtual void Finalize() override {_list.Finalize();}
};

class CRActionRunFunction : public CRAction {
  private:
    CRInternalFunction _function;
    std::vector<CRExpression> _args;

  public:
    CRActionRunFunction(CRInternalFunction func, std::vector<CRExpression>& args) : _function(func), _args(args) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        for (CRExpression& exp : _args) {
            exp.Finalize();
        }
    }
};

class CRActionRunScript : public CRAction {
  private:
    unsigned int _scriptID;
    std::vector<CRExpression> _args;

  public:
    CRActionRunScript(unsigned int id, std::vector<CRExpression>& args) : _scriptID(id), _args(args) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        for (CRExpression& exp : _args) {
            exp.Finalize();
        }
    }
};

class CRActionIfElse : public CRAction {
  private:
    CRAction* _if;
    CRAction* _else;
    CRExpression _expression;

  public:
    CRActionIfElse(CRAction* i, CRAction* e, CRExpression exp) : _if(i), _else(e), _expression(exp) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _if->Finalize();
        delete _if;
        if(_else) {
            _else->Finalize();
            delete _else;
        }
        _expression.Finalize();
    }
};

class CRActionWith : public CRAction {
  private:
    CRExpression _expression;
    CRAction* _code;

  public:
    CRActionWith(CRExpression exp, CRAction* code) : _expression(exp), _code(code) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _code->Finalize();
        delete _code;
        _expression.Finalize();
    }
};

class CRActionRepeat : public CRAction {
  private:
    CRExpression _expression;
    CRAction* _code;

  public:
    CRActionRepeat(CRExpression exp, CRAction* code) : _expression(exp), _code(code) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _code->Finalize();
        delete _code;
        _expression.Finalize();
    }
};

class CRActionWhile : public CRAction {
  private:
    CRExpression _expression;
    CRAction* _code;

  public:
    CRActionWhile(CRExpression exp, CRAction* code) : _expression(exp), _code(code) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _code->Finalize();
        delete _code;
        _expression.Finalize();
    }
};

class CRActionFor : public CRAction {
  private:
    CRAction* _initializer;
    CRExpression _check;
    CRAction* _finalizer;
    CRAction* _code;

  public:
    CRActionFor(CRAction* init, CRExpression exp, CRAction* final, CRAction* code) : _initializer(init), _check(exp), _finalizer(final), _code(code) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _initializer->Finalize();
        delete _initializer;
        _finalizer->Finalize();
        delete _finalizer;
        _code->Finalize();
        delete _code;
        _check.Finalize();
    }
};

class CRActionDoUntil : public CRAction {
  private:
    CRExpression _expression;
    CRAction* _code;

  public:
    CRActionDoUntil(CRExpression exp, CRAction* code) : _expression(exp), _code(code) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _code->Finalize();
        delete _code;
        _expression.Finalize();
    }
};

struct SwitchCase {
    CRExpression expression;
    unsigned int offset;
    SwitchCase(CRExpression exp, unsigned int off) : expression(exp), offset(off) {}
};

class CRActionSwitch : public CRAction {
  private:
    CRExpression _expression;
    CRActionList _actions;
    std::vector<SwitchCase> _cases;
    unsigned int _defaultOffset;

  public:
    CRActionSwitch(CRExpression exp, CRActionList actions, std::vector<SwitchCase>& offsets, unsigned int def) : _expression(exp), _actions(actions), _cases(offsets), _defaultOffset(def) {}
    virtual bool Run() override;
    virtual void Finalize() override {
        _expression.Finalize();
        _actions.Finalize();
        for(SwitchCase& c : _cases) {
            c.expression.Finalize();
        }
    }
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
    virtual void Finalize() override {_expression.Finalize();}
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
    void Finalize() override {
        for(CRExpression& arg : _args) {
            arg.Finalize();
        }
    }
};

class CRExpScript : public CRExpressionValue {
  private:
    unsigned int _script;
    std::vector<CRExpression> _args;

  public:
    CRExpScript(unsigned int id, std::vector<CRExpression>& args) : _script(id), _args(args) {}
    bool _evaluate(GMLType* output) override;
    void Finalize() override {
        for (CRExpression& arg : _args) {
            arg.Finalize();
        }
    }
};

class CRExpNestedExpression : public CRExpressionValue {
  private:
    CRExpression _expression;

  public:
    CRExpNestedExpression(CRExpression exp) : _expression(exp) {}
    bool _evaluate(GMLType* output) override;
    void Finalize() override {_expression.Finalize();}
};

class CRExpField : public CRExpressionValue {
  private:
    unsigned int _fieldNumber;
    CRExpression _deref;
    bool _hasDeref;
    bool _isLocal;

  public:
    CRExpField(unsigned int field, bool isLocal) : _fieldNumber(field), _hasDeref(false), _isLocal(isLocal) {}
    CRExpField(unsigned int field, CRExpression deref, bool isLocal) : _fieldNumber(field), _deref(deref), _hasDeref(true), _isLocal(isLocal) {}
    bool _evaluate(GMLType* output) override;
    void Finalize() override { _deref.Finalize(); }
};

class CRExpArray : public CRExpressionValue {
  private:
    unsigned int _fieldNumber;
    std::vector<CRExpression> _dimensions;
    CRExpression _deref;
    bool _hasDeref;
    bool _isLocal;

  public:
    CRExpArray(unsigned int field, std::vector<CRExpression>& dimensions, bool isLocal) : _fieldNumber(field), _dimensions(dimensions), _hasDeref(false), _isLocal(isLocal) {}
    CRExpArray(unsigned int field, std::vector<CRExpression>& dimensions, CRExpression deref, bool isLocal) : _fieldNumber(field), _dimensions(dimensions), _deref(deref), _hasDeref(true), _isLocal(isLocal) {}
    bool _evaluate(GMLType* output) override;
    void Finalize() override {
        for (CRExpression& arg : _dimensions) {
            arg.Finalize();
        }
        _deref.Finalize();
    }
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
    void Finalize() override {
        for (CRExpression& arg : _dimensions) {
            arg.Finalize();
        }
        _deref.Finalize();
    }
};

class CRExpGameVar : public CRExpressionValue {
  private:
    CRGameVar _var;
    std::vector<CRExpression> _dimensions;

  public:
    CRExpGameVar(CRGameVar var, std::vector<CRExpression>& dimensions) : _var(var), _dimensions(dimensions) {}
    bool _evaluate(GMLType* output) override;
    void Finalize() override {
        for (CRExpression& arg : _dimensions) {
            arg.Finalize();
        }
    }
};

#endif