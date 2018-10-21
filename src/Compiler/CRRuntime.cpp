#include <pch.h>
#include "CRRuntime.hpp"
#include "Compiled.hpp"
#include "Instance.hpp"
#include "InstanceList.hpp"

GlobalValues* _globals;

void Runtime::Init(GlobalValues* globals) {
    _globals = globals;
}

void Runtime::Finalize() {
}

GlobalValues* Runtime::GetGlobals() {
    return _globals;
}


int Runtime::_round(double d) {
    // This mimics the x86_32 "FISTP" operator which is commonly used in the GM8 runner.
    // We can't actually use that operator, because we're targeting other platforms than x86 Windows.
    int down = ( int )d;
    if ((d - down) < 0.5) return down;
    if ((d - down) > 0.5) return (down + 1);
    return down + (down & 1);
}

bool Runtime::_equal(double d1, double d2) {
    // I have no idea why GM8 does this, but it does.
    double difference = fabs(d2 - d1);
    double cut_digits = ::floor(difference * 1e13) / 1e13;
    return cut_digits == 0.0;
}

bool Runtime::_isTrue(const GMLType* value) {
    return (value->state == GMLTypeState::Double) && (value->dVal >= 0.5);
}

Runtime::Context _context;
Runtime::Context Runtime::GetContext() {
    return _context;
}


bool _applySetMethod(GMLType* lhs, CRSetMethod method, const GMLType* const rhs) {
    if (method == SM_ASSIGN) {
        // Easiest method
        (*lhs) = (*rhs);
        return true;
    }
    else if (method == SM_ADD) {
        // Only other method that can be used on strings
        if ((lhs->state == GMLTypeState::String) != (rhs->state == GMLTypeState::String)) {
            // Incompatible operands
            return false;
        }
        if (lhs->state == GMLTypeState::String) {
            lhs->sVal += rhs->sVal;
        }
        else {
            lhs->dVal += rhs->dVal;
        }
        return true;
    }
    else {
        // No other set methods can be used with strings, so we can error if either one is a string
        if ((lhs->state == GMLTypeState::String) || (rhs->state == GMLTypeState::String)) {
            return false;
        }
        switch (method) {
            case SM_SUBTRACT:
                lhs->dVal -= rhs->dVal;
                break;
            case SM_MULTIPLY:
                lhs->dVal *= rhs->dVal;
                break;
            case SM_DIVIDE:
                lhs->dVal /= rhs->dVal;
                break;
            case SM_BITWISE_AND:
                lhs->dVal = ( double )(Runtime::_round(lhs->dVal) & Runtime::_round(rhs->dVal));
                break;
            case SM_BITWISE_OR:
                lhs->dVal = ( double )(Runtime::_round(lhs->dVal) | Runtime::_round(rhs->dVal));
                break;
            case SM_BITWISE_XOR:
                lhs->dVal = ( double )(Runtime::_round(lhs->dVal) ^ Runtime::_round(rhs->dVal));
                break;
        }
        return true;
    }
}


bool CRActionList::Run(unsigned int start) {
    auto i = _actions.begin() + start;
    for (CRAction* action = *i; i != _actions.end(); i++) {
        if (!action->Run()) return false;
    }
    return true;
}

bool CRExpression::Evaluate(GMLType* output) {
    if (!_values.size()) return false;

    GMLType var;
    if (!_values[0]->Evaluate(&var)) return false;
    auto i = _values.begin();
    while ((i + 1) != _values.end()) {
        GMLType rhs;
        if (!(*(i + 1))->Evaluate(&rhs)) return false;

        switch ((*i)->GetOperator()) {
            case OPERATOR_ADD: {
                if (!_applySetMethod(&var, SM_ADD, &rhs)) return false;
                break;
            }
            case OPERATOR_SUBTRACT: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal -= rhs.dVal;
                break;
            }
            case OPERATOR_MULTIPLY: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal *= rhs.dVal;
                break;
            }
            case OPERATOR_DIVIDE: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal /= rhs.dVal;
                break;
            }
            case OPERATOR_MOD: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = std::fmod(var.dVal, rhs.dVal);
                break;
            }
            case OPERATOR_DIV: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = ::floor(var.dVal / rhs.dVal);
                break;
            }
            case OPERATOR_LTE: {
                if (var.state == GMLTypeState::Double) {
                    var.dVal = ((var.dVal < rhs.dVal || Runtime::_equal(var.dVal, rhs.dVal)) ? GMLTrue : GMLFalse);
                }
                else {
                    var.dVal = (var.sVal.length() <= rhs.sVal.length() ? GMLTrue : GMLFalse);
                }
                var.state = GMLTypeState::Double;
                break;
            }
            case OPERATOR_GTE: {
                if (var.state == GMLTypeState::Double) {
                    var.dVal = ((var.dVal > rhs.dVal || Runtime::_equal(var.dVal, rhs.dVal)) ? GMLTrue : GMLFalse);
                }
                else {
                    var.dVal = (var.sVal.length() >= rhs.sVal.length() ? GMLTrue : GMLFalse);
                }
                var.state = GMLTypeState::Double;
                break;
            }
            case OPERATOR_LT: {
                if (var.state == GMLTypeState::Double) {
                    var.dVal = (var.dVal < rhs.dVal ? GMLTrue : GMLFalse);
                }
                else {
                    var.dVal = (var.sVal.length() < rhs.sVal.length() ? GMLTrue : GMLFalse);
                }
                var.state = GMLTypeState::Double;
                break;
            }
            case OPERATOR_GT: {
                if (var.state == GMLTypeState::Double) {
                    var.dVal = (var.dVal > rhs.dVal ? GMLTrue : GMLFalse);
                }
                else {
                    var.dVal = (var.sVal.length() > rhs.sVal.length() ? GMLTrue : GMLFalse);
                }
                var.state = GMLTypeState::Double;
                break;
            }
            case OPERATOR_EQUALS: {
                if (var.state == GMLTypeState::Double)
                    var.dVal = (Runtime::_equal(var.dVal, rhs.dVal) ? GMLTrue : GMLFalse);
                else
                    var.dVal = (var.sVal.compare(rhs.sVal) ? GMLFalse : GMLTrue);
                var.state = GMLTypeState::Double;
                break;
            }
            case OPERATOR_NOT_EQUAL: {
                if (var.state == GMLTypeState::Double)
                    var.dVal = (Runtime::_equal(var.dVal, rhs.dVal) ? GMLFalse : GMLTrue);
                else
                    var.dVal = (var.sVal.compare(rhs.sVal) ? GMLTrue : GMLFalse);
                var.state = GMLTypeState::Double;
                break;
            }
            case OPERATOR_BOOLEAN_AND: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = (Runtime::_isTrue(&var) && Runtime::_isTrue(&rhs) ? GMLTrue : GMLFalse);
                break;
            }
            case OPERATOR_BOOLEAN_OR: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = (Runtime::_isTrue(&var) || Runtime::_isTrue(&rhs) ? GMLTrue : GMLFalse);
                break;
            }
            case OPERATOR_BOOLEAN_XOR: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = (Runtime::_isTrue(&var) != Runtime::_isTrue(&rhs) ? GMLTrue : GMLFalse);
                break;
            }
            case OPERATOR_BITWISE_AND: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = ( double )(Runtime::_round(var.dVal) & Runtime::_round(rhs.dVal));
                break;
            }
            case OPERATOR_BITWISE_OR: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = ( double )(Runtime::_round(var.dVal) | Runtime::_round(rhs.dVal));
                break;
            }
            case OPERATOR_BITWISE_XOR: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = ( double )(Runtime::_round(var.dVal) ^ Runtime::_round(rhs.dVal));
                break;
            }
            case OPERATOR_LSHIFT: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = ( double )(Runtime::_round(var.dVal) << Runtime::_round(rhs.dVal));
                break;
            }
            case OPERATOR_RSHIFT: {
                if (var.state == GMLTypeState::String) return false;
                var.dVal = ( double )(Runtime::_round(var.dVal) >> Runtime::_round(rhs.dVal));
                break;
            }
            default: { return false; }
        }
        i++;
    }
    (*output) = var;
    return true;
}

bool CRActionAssignmentField::Run() {
    if (_hasDeref) {
        GMLType v, d;
        if (!_expression.Evaluate(&v)) return false;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case -1:
            case -2:
            case -3:
            case -4:
            case -5:
            case -7:
                // todo
                return false;
            default:
                InstanceList::Iterator iter(( unsigned int )id);
                Instance* i;
                while (i = iter.Next()) {
                    if (!_applySetMethod(InstanceList::GetField(i->id, _field), _method, &v)) return false;
                }
        }
    }
    else {
        // todo, requires knowing the "self" which is a runtime context variable
        return false;
    }
    return true;
}

bool CRActionAssignmentArray::Run() { return false; }

bool CRActionAssignmentInstanceVar::Run() { return false; }

bool CRActionAssignmentGameVar::Run() { return false; }

bool CRActionBlock::Run() { return false; }

bool CRActionRunFunction::Run() { return false; }

bool CRActionRunScript::Run() { return false; }

bool CRActionIfElse::Run() { return false; }

bool CRActionWith::Run() { return false; }

bool CRActionSwitch::Run() { return false; }

bool CRExpLiteral::Evaluate(GMLType* output) { return false; }

bool CRExpFunction::Evaluate(GMLType* output) { return false; }

bool CRExpScript::Evaluate(GMLType* output) { return false; }

bool CRExpNestedExpression::Evaluate(GMLType* output) { return false; }

bool CRExpField::Evaluate(GMLType* output) { return false; }

bool CRExpArray::Evaluate(GMLType* output) { return false; }

bool CRExpInstanceVar::Evaluate(GMLType* output) { return false; }

bool CRExpGameVar::Evaluate(GMLType* output) { return false; }