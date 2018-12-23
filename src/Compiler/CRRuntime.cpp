#include <pch.h>

#include <chrono>
#include "Alarm.hpp"
#include "AssetManager.hpp"
#include "CRRuntime.hpp"
#include "CodeRunner.hpp"
#include "Collision.hpp"
#include "Compiled.hpp"
#include "GlobalValues.hpp"
#include "Instance.hpp"
#include "InstanceList.hpp"
#include "Renderer.hpp"

GlobalValues* _globalValues;
std::map<unsigned int,  std::map<unsigned int, GMLType>> _global;
std::map<CRInstanceVar, std::map<unsigned int, GMLType>> _globalInstance;
std::vector<bool (*)(unsigned int, GMLType*, GMLType*)> _gmlFuncs;
std::string _error;

void Runtime::Init(GlobalValues* globals, std::vector<bool (*)(unsigned int, GMLType*, GMLType*)>& gmlFuncs) {
    _globalValues = globals;
    _gmlFuncs = gmlFuncs;
}

void Runtime::Finalize() {}

GlobalValues* Runtime::GetGlobals() { return _globalValues; }


Runtime::Context _context;
Runtime::Context Runtime::GetContext() { return _context; }

GMLType returnBuffer;
Runtime::ReturnCause _cause;
Runtime::ReturnCause Runtime::GetReturnCause() { return _cause; }
void Runtime::SetReturnCause(Runtime::ReturnCause c) { _cause = c; }


// For verifying varargs
bool Runtime::_assertArgs(unsigned int& argc, GMLType* argv, unsigned int arge, bool lenient, ...) {
    if (argc != arge) {
        _cause = ReturnCause::ExitError;
        _error = "Failed to verify args: wrong number: expected " + std::to_string(arge) + ", got " + std::to_string(argc);
        return false;
    }

    va_list vargs;
    va_start(vargs, lenient);

    for (size_t i = 0; i < argc; i++) {
        GMLTypeState state = va_arg(vargs, GMLTypeState);
        if (argv[i].state != state) {
            if (lenient && (argv[i].state == GMLTypeState::String)) {
                argv[i].state = GMLTypeState::Double;
                argv[i].dVal = 0.0;
            }
            else {
                _cause = ReturnCause::ExitError;
                _error = "Failed to verify args: wrong type in position " + std::to_string(i) + ": expected ";
                _error += (state == GMLTypeState::String ? "string" : "real");
                _error += ", got ";
                _error += (argv[i].state == GMLTypeState::String ? "string" : "real");
                return false;
            }
        }
    }
    va_end(vargs);
    return true;
}

bool CRExpressionValue::Evaluate(GMLType* output) {
    if (!this->_evaluate(output)) return false;
    for (CRUnaryOperator op : _unary) {
        if (output->state == GMLTypeState::String) {
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Tried to apply unary operator " + std::to_string(op) + " to string \"" + output->sVal + "\"";
            return false;
        }
        switch (op) {
            case OPERATOR_NOT:
                output->dVal = (Runtime::_isTrue(output) ? GMLFalse : GMLTrue);
                break;
            case OPERATOR_TILDE:
                output->dVal = ~Runtime::_round(output->dVal);
                break;
            case OPERATOR_NEGATIVE:
                output->dVal = -output->dVal;
                break;
            case OPERATOR_POSITIVE:
                break;
            default:
                _cause = Runtime::ReturnCause::ExitError;
                _error = "Unrecognized unary operator " + std::to_string(op);
                return false;
        }
    }
    return true;
}


bool _applySetMethod(GMLType* lhs, CRSetMethod method, const GMLType* const rhs) {
    if (method == SM_ASSIGN) {
        // Easiest method
        (*lhs) = (*rhs);
        return true;
    }
    else if (method == SM_ADD) {
        // Only other method that can be used on strings
        if (lhs->state != rhs->state) {
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Incompatible operands for +, lhs: ";
            _error += (lhs->state == GMLTypeState::Double) ? std::to_string(lhs->dVal) : ("\"" + lhs->sVal + "\"");
            _error += ", rhs: " + (rhs->state == GMLTypeState::Double) ? std::to_string(rhs->dVal) : ("\"" + rhs->sVal + "\"");
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
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Incompatible operands for method " + std::to_string(method) + ", lhs: ";
            _error += (lhs->state == GMLTypeState::Double) ? std::to_string(lhs->dVal) : ("\"" + lhs->sVal + "\"");
            _error += ", rhs: " + (rhs->state == GMLTypeState::Double) ? std::to_string(rhs->dVal) : ("\"" + rhs->sVal + "\"");
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


bool _getGameValue(CRGameVar index, unsigned int arrayIndex, GMLType* out) {
    out->state = GMLTypeState::Double;
    switch (index) {
        case ARGUMENT:
            if (_context.argc > arrayIndex) {
                (*out) = _context.argv[arrayIndex];
            }
            break;
        case ARGUMENT0:
            if (_context.argc > 0) {
                (*out) = _context.argv[0];
            }
            break;
        case ARGUMENT1:
            if (_context.argc > 1) {
                (*out) = _context.argv[1];
            }
            break;
        case ARGUMENT2:
            if (_context.argc > 2) {
                (*out) = _context.argv[2];
            }
            break;
        case ARGUMENT3:
            if (_context.argc > 3) {
                (*out) = _context.argv[3];
            }
            break;
        case ARGUMENT4:
            if (_context.argc > 4) {
                (*out) = _context.argv[4];
            }
            break;
        case ARGUMENT5:
            if (_context.argc > 5) {
                (*out) = _context.argv[5];
            }
            break;
        case ARGUMENT6:
            if (_context.argc > 6) {
                (*out) = _context.argv[6];
            }
            break;
        case ARGUMENT7:
            if (_context.argc > 7) {
                (*out) = _context.argv[7];
            }
            break;
        case ARGUMENT8:
            if (_context.argc > 8) {
                (*out) = _context.argv[8];
            }
            break;
        case ARGUMENT9:
            if (_context.argc > 9) {
                (*out) = _context.argv[9];
            }
            break;
        case ARGUMENT10:
            if (_context.argc > 10) {
                (*out) = _context.argv[10];
            }
            break;
        case ARGUMENT11:
            if (_context.argc > 11) {
                (*out) = _context.argv[11];
            }
            break;
        case ARGUMENT12:
            if (_context.argc > 12) {
                (*out) = _context.argv[12];
            }
            break;
        case ARGUMENT13:
            if (_context.argc > 13) {
                (*out) = _context.argv[13];
            }
            break;
        case ARGUMENT14:
            if (_context.argc > 14) {
                (*out) = _context.argv[14];
            }
            break;
        case ARGUMENT15:
            if (_context.argc > 15) {
                (*out) = _context.argv[15];
            }
            break;
        case CURRENT_TIME: {
            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
            unsigned long time = (unsigned long)(t1.time_since_epoch() / std::chrono::milliseconds(1));
            out->dVal = static_cast<double>(time);
            break;
        }
        case INSTANCE_COUNT:
            out->dVal = static_cast<double>(InstanceList::Count());
            break;
        case MOUSE_X:
            int mx;
            RGetCursorPos(&mx, NULL);
            out->dVal = static_cast<double>(mx);
            break;
        case MOUSE_Y:
            int my;
            RGetCursorPos(NULL, &my);
            out->dVal = static_cast<double>(my);
            break;
        case ROOM:
            out->dVal = static_cast<double>(_globalValues->room);
            break;
        case ROOM_SPEED:
            out->dVal = static_cast<double>(_globalValues->room_speed);
            break;
        case ROOM_WIDTH:
            out->dVal = static_cast<double>(_globalValues->room_width);
            break;
        case ROOM_HEIGHT:
            out->dVal = static_cast<double>(_globalValues->room_height);
            break;
        case ROOM_CAPTION:
            out->state = GMLTypeState::String;
            out->sVal = _globalValues->room_caption;
            break;
        case VIEW_ENABLED:
            out->dVal = _globalValues->view_enabled ? GMLTrue : GMLFalse;
            break;
        case VIEW_VISIBLE:
            out->dVal = _globalValues->views[arrayIndex].visible ? GMLTrue : GMLFalse;
            break;
        case VIEW_XVIEW:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].xview);
            break;
        case VIEW_YVIEW:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].yview);
            break;
        case VIEW_WVIEW:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].wview);
            break;
        case VIEW_HVIEW:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].hview);
            break;
        case VIEW_XPORT:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].xport);
            break;
        case VIEW_YPORT:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].yport);
            break;
        case VIEW_WPORT:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].wport);
            break;
        case VIEW_HPORT:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].hport);
            break;
        case VIEW_ANGLE:
            out->dVal = _globalValues->views[arrayIndex].angle;
            break;
        case VIEW_HBORDER:
            out->dVal = _globalValues->views[arrayIndex].hborder;
            break;
        case VIEW_VBORDER:
            out->dVal = _globalValues->views[arrayIndex].vborder;
            break;
        case VIEW_HSPEED:
            out->dVal = _globalValues->views[arrayIndex].hspeed;
            break;
        case VIEW_VSPEED:
            out->dVal = _globalValues->views[arrayIndex].vspeed;
            break;
        case VIEW_OBJECT:
            out->dVal = static_cast<double>(_globalValues->views[arrayIndex].object);
            break;
        default:
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Read unrecognized Game Var " + std::to_string(index);
            return false;
    }
    return true;
}

bool _setGameValue(CRGameVar index, unsigned int arrayIndex, CRSetMethod method, GMLType& value) {
    GMLType lhs;
    lhs.state = GMLTypeState::Double;
    switch (index) {
        case ROOM:
            lhs.dVal = static_cast<double>(_globalValues->roomTarget);
            if(!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->roomTarget = static_cast<unsigned int>(Runtime::_round(lhs.dVal));
            _globalValues->changeRoom = true;
            break;
        case ROOM_SPEED:
            lhs.dVal = static_cast<double>(_globalValues->room_speed);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->room_speed = static_cast<unsigned int>(Runtime::_round(lhs.dVal));
            break;
        case ROOM_CAPTION:
            lhs.state = GMLTypeState::String;
            lhs.sVal = _globalValues->room_caption;
            if(!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->room_caption = lhs.sVal;
            break;
        case VIEW_ENABLED:
            lhs.dVal = _globalValues->view_enabled ? GMLTrue : GMLFalse;
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->view_enabled = Runtime::_isTrue(&lhs);
            break;
        case VIEW_VISIBLE:
            lhs.dVal = _globalValues->views[arrayIndex].visible ? GMLTrue : GMLFalse;
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].visible = Runtime::_isTrue(&lhs);
            break;
        case VIEW_XVIEW:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].xview);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].xview = static_cast<int>(lhs.dVal);
            break;
        case VIEW_YVIEW:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].yview);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].yview = static_cast<int>(lhs.dVal);
            break;
        case VIEW_WVIEW:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].wview);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].wview = static_cast<int>(lhs.dVal);
            break;
        case VIEW_HVIEW:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].hview);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].hview = static_cast<int>(lhs.dVal);
            break;
        case VIEW_XPORT:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].xport);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].xport = static_cast<int>(lhs.dVal);
            break;
        case VIEW_YPORT:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].yport);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].yport = static_cast<int>(lhs.dVal);
            break;
        case VIEW_WPORT:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].wport);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].wport = static_cast<int>(lhs.dVal);
            break;
        case VIEW_HPORT:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].hport);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].hport = static_cast<int>(lhs.dVal);
            break;
        case VIEW_ANGLE:
            lhs.dVal = _globalValues->views[arrayIndex].angle;
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].angle = lhs.dVal;
            break;
        case VIEW_HBORDER:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].hborder);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].hborder = static_cast<int>(lhs.dVal);
            break;
        case VIEW_VBORDER:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].vborder);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].vborder = static_cast<int>(lhs.dVal);
            break;
        case VIEW_HSPEED:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].hspeed);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].hspeed = static_cast<int>(lhs.dVal);
            break;
        case VIEW_VSPEED:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].vspeed);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].vspeed = static_cast<int>(lhs.dVal);
            break;
        case VIEW_OBJECT:
            lhs.dVal = static_cast<double>(_globalValues->views[arrayIndex].object);
            if (!_applySetMethod(&lhs, method, &value)) return false;
            _globalValues->views[arrayIndex].object = static_cast<int>(lhs.dVal);
            break;
        default:
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Tried to write unrecognized or read-only Game Var " + std::to_string(index);
            return false;
    }
    return true;
}


bool _setInstanceVar(Instance& instance, CRInstanceVar index, unsigned int arrayIndex, CRSetMethod method, GMLType value) {
    // No instance vars are strings. In GML if you set an instance var to a string, it gets set to 0.
    GMLType t;
    t.state = GMLTypeState::Double;
    if (value.state != GMLTypeState::Double) {
        value.state = GMLTypeState::Double;
        value.dVal = 0;
    }

    switch (index) {
        case IV_ALARM: {
            int alarmValue = (value.state == GMLTypeState::Double ? Runtime::_round(value.dVal) : 0);
            if (alarmValue)
                AlarmSet(instance.id, static_cast<unsigned int>(arrayIndex), alarmValue);
            else
                AlarmDelete(instance.id, static_cast<unsigned int>(arrayIndex));
            break;
        }
        case IV_DIRECTION:
            t.dVal = instance.direction;
            if (!_applySetMethod(&t, method, &value)) return false;
            while (t.dVal >= 360.0) t.dVal -= 360.0;
            while (t.dVal < 0.0) t.dVal += 360.0;
            instance.direction = (t.dVal);
            instance.hspeed = ::cos(instance.direction * GML_PI / 180.0) * instance.speed;
            instance.vspeed = -::sin(instance.direction * GML_PI / 180.0) * instance.speed;
            break;
        case IV_IMAGE_SPEED:
            t.dVal = instance.image_speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.image_speed = t.dVal;
            break;
        case IV_FRICTION:
            t.dVal = instance.friction;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.friction = t.dVal;
            break;
        case IV_SPRITE_INDEX:
            t.dVal = instance.sprite_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.sprite_index = Runtime::_round(t.dVal);
            instance.bboxIsStale = true;
            break;
        case IV_MASK_INDEX:
            t.dVal = instance.mask_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.mask_index = Runtime::_round(t.dVal);
            instance.bboxIsStale = true;
            break;
        case IV_IMAGE_BLEND:
            t.dVal = instance.image_blend;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.image_blend = Runtime::_round(t.dVal);
            break;
        case IV_IMAGE_ALPHA:
            t.dVal = instance.image_alpha;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.image_alpha = t.dVal;
            break;
        case IV_IMAGE_INDEX:
            t.dVal = instance.image_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.image_index = t.dVal;
            instance.bboxIsStale = true;
            break;
        case IV_IMAGE_ANGLE:
            t.dVal = instance.image_angle;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.image_angle = t.dVal;
            instance.bboxIsStale = true;
            break;
        case IV_IMAGE_XSCALE:
            t.dVal = instance.image_xscale;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.image_xscale = t.dVal;
            instance.bboxIsStale = true;
            break;
        case IV_IMAGE_YSCALE:
            t.dVal = instance.image_yscale;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.image_yscale = t.dVal;
            instance.bboxIsStale = true;
            break;
        case IV_SOLID:
            t.dVal = (instance.solid ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.solid = Runtime::_isTrue(&t);
            break;
        case IV_VISIBLE:
            t.dVal = (instance.visible ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.visible = Runtime::_isTrue(&t);
            break;
        case IV_PERSISTENT:
            t.dVal = (instance.persistent ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.persistent = Runtime::_isTrue(&t);
            break;
        case IV_DEPTH:
            t.dVal = instance.depth;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.depth = Runtime::_round(t.dVal);
            break;
        case IV_SPEED:
            t.dVal = instance.speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.speed = t.dVal;
            instance.hspeed = ::cos(instance.direction * GML_PI / 180.0) * instance.speed;
            instance.vspeed = -::sin(instance.direction * GML_PI / 180.0) * instance.speed;
            break;
        case IV_VSPEED:
            t.dVal = instance.vspeed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.vspeed = t.dVal;
            instance.direction = ::atan2(-instance.vspeed, instance.hspeed) * 180.0 / GML_PI;
            instance.speed = ::sqrt(pow(instance.hspeed, 2) + pow(instance.vspeed, 2));
            break;
        case IV_HSPEED:
            t.dVal = instance.hspeed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.hspeed = t.dVal;
            instance.direction = ::atan2(-instance.vspeed, instance.hspeed) * 180.0 / GML_PI;
            instance.speed = ::sqrt(pow(instance.hspeed, 2) + pow(instance.vspeed, 2));
            break;
        case IV_GRAVITY:
            t.dVal = instance.gravity;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.gravity = t.dVal;
            break;
        case IV_GRAVITY_DIRECTION:
            t.dVal = instance.gravity_direction;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.gravity_direction = t.dVal;
            break;
        case IV_X:
            t.dVal = instance.x;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.x = t.dVal;
            instance.bboxIsStale = true;
            break;
        case IV_Y:
            t.dVal = instance.y;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.y = t.dVal;
            instance.bboxIsStale = true;
            break;
        case IV_PATH_INDEX:
            t.dVal = instance.path_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.path_index = Runtime::_round(t.dVal);
            break;
        case IV_PATH_POSITION:
            t.dVal = instance.path_position;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.path_position = t.dVal;
            break;
        case IV_PATH_SPEED:
            t.dVal = instance.path_speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.path_speed = t.dVal;
            break;
        case IV_PATH_SCALE:
            t.dVal = instance.path_scale;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.path_scale = t.dVal;
            break;
        case IV_PATH_ORIENTATION:
            t.dVal = instance.path_orientation;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.path_orientation = t.dVal;
            break;
        case IV_PATH_ENDACTION:
            t.dVal = instance.path_endaction;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.path_endaction = Runtime::_round(t.dVal);
            break;
        case IV_TIMELINE_INDEX:
            t.dVal = instance.timeline_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.timeline_index = Runtime::_round(t.dVal);
            break;
        case IV_TIMELINE_RUNNING:
            t.dVal = (instance.timeline_running ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.timeline_running = Runtime::_isTrue(&t);
            break;
        case IV_TIMELINE_LOOP:
            t.dVal = (instance.timeline_loop ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.timeline_loop = Runtime::_isTrue(&t);
            break;
        case IV_TIMELINE_SPEED:
            t.dVal = instance.timeline_speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.timeline_speed = t.dVal;
            break;
        case IV_TIMELINE_POSITION:
            t.dVal = instance.timeline_position;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance.timeline_position = t.dVal;
            break;
        default:
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Tried to write unrecognized or read-only instance var " + std::to_string(index);
            return false;
    }
    return true;
}

bool _getInstanceVar(Instance& instance, CRInstanceVar index, unsigned int arrayIndex, GMLType* out) {
    out->state = GMLTypeState::Double;
    switch (index) {
        case IV_ALARM:
            out->dVal = static_cast<double>(AlarmGet(instance.id, arrayIndex));
            break;
        case IV_INSTANCE_ID:
            out->dVal = instance.id;
            break;
        case IV_OBJECT_INDEX:
            out->dVal = instance.object_index;
            break;
        case IV_X:
            out->dVal = instance.x;
            break;
        case IV_Y:
            out->dVal = instance.y;
            break;
        case IV_XPREVIOUS:
            out->dVal = instance.xprevious;
            break;
        case IV_YPREVIOUS:
            out->dVal = instance.yprevious;
            break;
        case IV_XSTART:
            out->dVal = instance.xstart;
            break;
        case IV_YSTART:
            out->dVal = instance.ystart;
            break;
        case IV_SOLID:
            out->dVal = (instance.solid ? GMLTrue : GMLFalse);
            break;
        case IV_DEPTH:
            out->dVal = static_cast<double>(instance.depth);
            break;
        case IV_VISIBLE:
            out->dVal = (instance.visible ? GMLTrue : GMLFalse);
            break;
        case IV_PERSISTENT:
            out->dVal = (instance.persistent ? GMLTrue : GMLFalse);
            break;
        case IV_DIRECTION:
            out->dVal = instance.direction;
            break;
        case IV_SPEED:
            out->dVal = instance.speed;
            break;
        case IV_VSPEED:
            out->dVal = instance.vspeed;
            break;
        case IV_HSPEED:
            out->dVal = instance.hspeed;
            break;
        case IV_GRAVITY:
            out->dVal = instance.gravity;
            break;
        case IV_GRAVITY_DIRECTION:
            out->dVal = instance.gravity_direction;
            break;
        case IV_FRICTION:
            out->dVal = instance.friction;
            break;
        case IV_IMAGE_INDEX:
            out->dVal = instance.image_index;
            break;
        case IV_IMAGE_SPEED:
            out->dVal = instance.image_speed;
            break;
        case IV_SPRITE_INDEX:
            out->dVal = instance.sprite_index;
            break;
        case IV_SPRITE_WIDTH:
            if (instance.sprite_index < 0 || instance.sprite_index >= static_cast<int>(AssetManager::GetSpriteCount())) {
                out->dVal = 0;
            }
            else {
                Sprite* s = AssetManager::GetSprite(instance.sprite_index);
                out->dVal = (s->exists ? s->width : 0);
            }
            break;
        case IV_SPRITE_HEIGHT:
            if (instance.sprite_index < 0 || instance.sprite_index >= static_cast<int>(AssetManager::GetSpriteCount())) {
                out->dVal = 0.0;
            }
            else {
                Sprite* s = AssetManager::GetSprite(instance.sprite_index);
                out->dVal = (s->exists ? s->height : 0);
            }
            break;
        case IV_MASK_INDEX:
            out->dVal = instance.mask_index;
            break;
        case IV_IMAGE_XSCALE:
            out->dVal = instance.image_xscale;
            break;
        case IV_IMAGE_YSCALE:
            out->dVal = instance.image_yscale;
            break;
        case IV_IMAGE_ANGLE:
            out->dVal = instance.image_angle;
            break;
        case IV_IMAGE_BLEND:
            out->dVal = static_cast<double>(instance.image_blend);
            break;
        case IV_IMAGE_ALPHA:
            out->dVal = instance.image_alpha;
            break;
        case IV_PATH_INDEX:
            out->dVal = static_cast<double>(instance.path_index);
            break;
        case IV_PATH_POSITION:
            out->dVal = instance.path_position;
            break;
        case IV_PATH_SPEED:
            out->dVal = instance.path_speed;
            break;
        case IV_PATH_SCALE:
            out->dVal = instance.path_scale;
            break;
        case IV_PATH_ORIENTATION:
            out->dVal = instance.path_orientation;
            break;
        case IV_PATH_ENDACTION:
            out->dVal = instance.path_endaction;
            break;
        case IV_PATH_POSITIONPREVIOUS:
            out->dVal = instance.path_positionprevious;
            break;
        case IV_TIMELINE_INDEX:
            out->dVal = static_cast<double>(instance.timeline_index);
            break;
        case IV_TIMELINE_RUNNING:
            out->dVal = instance.timeline_running ? GMLTrue : GMLFalse;
            break;
        case IV_TIMELINE_LOOP:
            out->dVal = instance.timeline_loop ? GMLTrue : GMLFalse;
            break;
        case IV_TIMELINE_SPEED:
            out->dVal = instance.timeline_speed;
            break;
        case IV_TIMELINE_POSITION:
            out->dVal = instance.timeline_position;
            break;
        case IV_BBOX_LEFT:
            RefreshInstanceBbox(&instance);
            out->dVal = instance.bbox_left;
            break;
        case IV_BBOX_RIGHT:
            RefreshInstanceBbox(&instance);
            out->dVal = instance.bbox_right;
            break;
        case IV_BBOX_BOTTOM:
            RefreshInstanceBbox(&instance);
            out->dVal = instance.bbox_bottom;
            break;
        case IV_BBOX_TOP:
            RefreshInstanceBbox(&instance);
            out->dVal = instance.bbox_top;
            break;
        default:
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Read unrecognized instance variable " + std::to_string(index);
            return false;
    }
    return true;
}


int Runtime::_round(double d) {
    // This mimics the x86_32 "FISTP" operator which is commonly used in the GM8 runner.
    // We can't actually use that operator, because we're targeting other platforms than x86 Windows.
    int down = static_cast<int>(d);
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

bool Runtime::_isTrue(const GMLType* value) { return (value->state == GMLTypeState::Double) && (value->dVal >= 0.5); }


const char* Runtime::GetErrorMessage() { return _error.c_str(); }

void Runtime::PushErrorMessage(const char* m) { _error += m; }

bool Runtime::Execute(CRActionList& actions, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, unsigned int argc, GMLType* argv) {
    Context c = _context;
    _context.self = self;
    _context.other = other;
    _context.eventId = ev;
    _context.eventNumber = sub;
    _context.objId = asObjId;
    _context.argc = argc;
    _context.argv = argv;
    bool result = actions.Run();
    _context = c;
    return result ? true : (_cause == ReturnCause::Break || _cause == ReturnCause::Continue || _cause == ReturnCause::Return || _cause == ReturnCause::ExitNormal);
}

bool Runtime::EvalExpression(CRExpression& expression, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, GMLType* out, unsigned int argc, GMLType* argv) {
    Context c = _context;
    _context.self = self;
    _context.other = other;
    _context.eventId = ev;
    _context.eventNumber = sub;
    _context.objId = asObjId;
    _context.argc = argc;
    _context.argv = argv;
    bool result = expression.Evaluate(out);
    _context = c;
    return result;
}


bool CRActionList::Run(unsigned int start) {
    auto i = _actions.begin() + start;
    while (i != _actions.end()) {
        if (!(*i)->Run()) return false;
        i++;
    }
    return true;
}

void CRActionList::Finalize() {
    for(CRAction* action : _actions) {
        action->Finalize();
        delete action;
    }
}

bool CRExpression::Evaluate(GMLType* output) {
    if (!_values.size()) {
        _cause = Runtime::ReturnCause::ExitError;
        _error = "Tried to evaluate empty expression";
        return false;
    }

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
                if (!_applySetMethod(&var, SM_SUBTRACT, &rhs)) return false;
                break;
            }
            case OPERATOR_MULTIPLY: {
                if (!_applySetMethod(&var, SM_MULTIPLY, &rhs)) return false;
                break;
            }
            case OPERATOR_DIVIDE: {
                if (!_applySetMethod(&var, SM_DIVIDE, &rhs)) return false;
                break;
            }
            case OPERATOR_MOD: {
                if (var.state == GMLTypeState::String || rhs.state == GMLTypeState::String) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Incompatible operands for operator mod: ";
                    _error += (var.state == GMLTypeState::Double) ? std::to_string(var.dVal) : ("\"" + var.sVal + "\"");
                    _error += ", rhs: " + (rhs.state == GMLTypeState::Double) ? std::to_string(rhs.dVal) : ("\"" + rhs.sVal + "\"");
                    return false;
                }
                var.dVal = std::fmod(var.dVal, rhs.dVal);
                break;
            }
            case OPERATOR_DIV: {
                if (var.state == GMLTypeState::String || rhs.state == GMLTypeState::String) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Incompatible operands for operator div: ";
                    _error += (var.state == GMLTypeState::Double) ? std::to_string(var.dVal) : ("\"" + var.sVal + "\"");
                    _error += ", rhs: " + (rhs.state == GMLTypeState::Double) ? std::to_string(rhs.dVal) : ("\"" + rhs.sVal + "\"");
                    return false;
                }
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
                var.dVal = (Runtime::_isTrue(&var) && Runtime::_isTrue(&rhs) ? GMLTrue : GMLFalse);
                break;
            }
            case OPERATOR_BOOLEAN_OR: {
                var.dVal = (Runtime::_isTrue(&var) || Runtime::_isTrue(&rhs) ? GMLTrue : GMLFalse);
                break;
            }
            case OPERATOR_BOOLEAN_XOR: {
                var.dVal = (Runtime::_isTrue(&var) != Runtime::_isTrue(&rhs) ? GMLTrue : GMLFalse);
                break;
            }
            case OPERATOR_BITWISE_AND: {
                if (!_applySetMethod(&var, SM_BITWISE_AND, &rhs)) return false;
                break;
            }
            case OPERATOR_BITWISE_OR: {
                if (!_applySetMethod(&var, SM_BITWISE_OR, &rhs)) return false;
                break;
            }
            case OPERATOR_BITWISE_XOR: {
                if (!_applySetMethod(&var, SM_BITWISE_XOR, &rhs)) return false;
                break;
            }
            case OPERATOR_LSHIFT: {
                if (var.state == GMLTypeState::String || rhs.state == GMLTypeState::String) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Incompatible operands for operator <<: ";
                    _error += (var.state == GMLTypeState::Double) ? std::to_string(var.dVal) : ("\"" + var.sVal + "\"");
                    _error += ", rhs: " + (rhs.state == GMLTypeState::Double) ? std::to_string(rhs.dVal) : ("\"" + rhs.sVal + "\"");
                    return false;
                }
                var.dVal = ( double )(Runtime::_round(var.dVal) << Runtime::_round(rhs.dVal));
                break;
            }
            case OPERATOR_RSHIFT: {
                if (var.state == GMLTypeState::String || rhs.state == GMLTypeState::String) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Incompatible operands for operator >>: ";
                    _error += (var.state == GMLTypeState::Double) ? std::to_string(var.dVal) : ("\"" + var.sVal + "\"");
                    _error += ", rhs: " + (rhs.state == GMLTypeState::Double) ? std::to_string(rhs.dVal) : ("\"" + rhs.sVal + "\"");
                    return false;
                }
                var.dVal = ( double )(Runtime::_round(var.dVal) >> Runtime::_round(rhs.dVal));
                break;
            }
            default:
                _cause = Runtime::ReturnCause::ExitError;
                _error = "Unrecognized operator: " + std::to_string((*i)->GetOperator());
                _error += (var.state == GMLTypeState::Double) ? std::to_string(var.dVal) : ("\"" + var.sVal + "\"");
                _error += ", rhs: " + (rhs.state == GMLTypeState::Double) ? std::to_string(rhs.dVal) : ("\"" + rhs.sVal + "\"");
                return false;
        }
        i++;
    }
    (*output) = var;
    return true;
}

void CRExpression::Finalize() {
    for(CRExpressionValue* value : _values) {
        value->Finalize();
        delete value;
    }
}

bool _evalArrayAccessor(std::vector<CRExpression>& dimensions, int* out) {
    if (dimensions.size()) {
        if (dimensions.size() > 2) {
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Tried to access " + std::to_string(dimensions.size()) + "-dimensional array; 2-dimensional is the highest supported";
            return false;
        }
        GMLType dim1;
        if (!dimensions[0].Evaluate(&dim1)) return false;
        if (dim1.state == GMLTypeState::String) {
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Invalid array accessor: \"" + dim1.sVal + "\"";
            return false;
        }
        (*out) = Runtime::_round(dim1.dVal);
        if ((*out) < 0 || (*out) >= 32000) {
            _cause = Runtime::ReturnCause::ExitError;
            _error = "Invalid array accessor: \"" + std::to_string(dim1.dVal) + "\"";
            return false;
        }

        if (dimensions.size() == 2) {
            GMLType dim2;
            if (!dimensions[1].Evaluate(&dim2)) return false;
            if (dim2.state == GMLTypeState::String) {
                _cause = Runtime::ReturnCause::ExitError;
                _error = "Invalid array accessor: \"" + dim1.sVal + "\"";
                return false;
            }
            int id2 = Runtime::_round(dim2.dVal);
            if (id2 < 0 || id2 >= 32000) {
                _cause = Runtime::ReturnCause::ExitError;
                _error = "Invalid array accessor: \"" + std::to_string(dim1.dVal) + "\"";
                return false;
            }
            (*out) = ((*out) * 32000) + id2;
        }
    }
    return true;
}


bool CRActionBindVars::Run() {
    return true;
}

bool CRActionAssignmentField::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                if (!_applySetMethod(InstanceList::GetField(_context.self, _field), _method, &v)) return false;
            }
            case OTHER: {
                if (!_applySetMethod(InstanceList::GetField(_context.other, _field), _method, &v)) return false;
            }
            case ALL: {
                InstanceList::Iterator iter;
                InstanceHandle i;
                while ((i = iter.Next()) != InstanceList::NoInstance) {
                    if (!_applySetMethod(InstanceList::GetField(i, _field), _method, &v)) return false;
                }
            }
            case GLOBAL: {
                if (!_applySetMethod(&_global[_field][0], _method, &v)) return false;
                break;
            }
            case LOCAL: {
                if (!_applySetMethod(&_context.locals[_field][0], _method, &v)) return false;
                break;
            }
            default: {
                if (id < 0) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Tried to dereference negative number: " + std::to_string(id);
                    return false;
                }
                InstanceList::Iterator iter(static_cast<unsigned int>(id));
                InstanceHandle i;
                while ((i = iter.Next()) != InstanceList::NoInstance) {
                    if (!_applySetMethod(InstanceList::GetField(i, _field), _method, &v)) return false;
                }
            }
        }
    }
    else {
        if (_isLocal) {
            // Local
            if (!_applySetMethod(&_context.locals[_field][0], _method, &v)) return false;
        }
        else {
            // Field
            if (!_applySetMethod(InstanceList::GetField(_context.self, _field), _method, &v)) return false;
        }
    }
    return true;
}

bool CRActionAssignmentArray::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;

    int index = 0;
    if (!_evalArrayAccessor(_dimensions, &index)) return false;

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                if (!_applySetMethod(InstanceList::GetField(_context.self, _field, index), _method, &v)) return false;
                break;
            }
            case OTHER: {
                if (!_applySetMethod(InstanceList::GetField(_context.other, _field, index), _method, &v)) return false;
                break;
            }
            case ALL: {
                InstanceList::Iterator iter;
                InstanceHandle i;
                while ((i = iter.Next()) != InstanceList::NoInstance) {
                    if (!_applySetMethod(InstanceList::GetField(i, _field, index), _method, &v)) return false;
                }
                break;
            }
            case GLOBAL: {
                if (!_applySetMethod(&_global[_field][index], _method, &v)) return false;
                break;
            }
            case LOCAL: {
                if (!_applySetMethod(&_context.locals[_field][index], _method, &v)) return false;
                break;
            }
            default: {
                if (id < 0) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Tried to dereference negative number: " + std::to_string(id);
                    return false;
                }
                InstanceList::Iterator iter(static_cast<unsigned int>(id));
                InstanceHandle i;
                while ((i = iter.Next()) != InstanceList::NoInstance) {
                    if (!_applySetMethod(InstanceList::GetField(i, _field, index), _method, &v)) return false;
                }
            }
        }
    }
    else {
        if (_isLocal) {
            // Local
            if (!_applySetMethod(&_context.locals[_field][index], _method, &v)) return false;
        }
        else {
            // Field
            if (!_applySetMethod(InstanceList::GetField(_context.self, _field, index), _method, &v)) return false;
        }
    }
    return true;
}

bool CRActionAssignmentInstanceVar::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;

    int index = 0;
    if (!_evalArrayAccessor(_dimensions, &index)) return false;

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                if (!_setInstanceVar(InstanceList::GetInstance(_context.self), _var, index, _method, v)) return false;
                break;
            }
            case OTHER: {
                if (!_setInstanceVar(InstanceList::GetInstance(_context.other), _var, index, _method, v)) return false;
                break;
            }
            case ALL: {
                InstanceList::Iterator iter;
                InstanceHandle i;
                while ((i = iter.Next()) != InstanceList::NoInstance) {
                    if (!_setInstanceVar(InstanceList::GetInstance(i), _var, index, _method, v)) return false;
                }
                break;
            }
            case GLOBAL: {
                if(!_applySetMethod(&_globalInstance[_var][index], _method, &v)) return false;
                return true;
            }
            case LOCAL: {
                if (!_applySetMethod(&_context.localInstance[_var][index], _method, &v)) return false;
                return true;
            }
            default: {
                if (id < 0) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Tried to dereference negative number: " + std::to_string(id);
                    return false;
                }
                InstanceList::Iterator iter(static_cast<unsigned int>(id));
                InstanceHandle i;
                while ((i = iter.Next()) != InstanceList::NoInstance) {
                    if (!_setInstanceVar(InstanceList::GetInstance(i), _var, index, _method, v)) return false;
                }
            }
        }
    }
    else {
        if (!_setInstanceVar(InstanceList::GetInstance(_context.self), _var, index, _method, v)) return false;
    }
    return true;
}

bool CRActionAssignmentGameVar::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;

    int index = 0;
    if (!_evalArrayAccessor(_dimensions, &index)) return false;

    return _setGameValue(_var, index, _method, v);
}

bool CRActionBlock::Run() { return _list.Run(); }

bool CRActionRunFunction::Run() {
    GMLType argv[16];
    unsigned int argc = static_cast<unsigned int>(_args.size());
    if (argc > 16) {
        _cause = Runtime::ReturnCause::ExitError;
        _error = "Too many args to internal function, argc: " + std::to_string(argc) + ", function ID: " + std::to_string(_function);
        return false;
    }
    for (unsigned int i = 0; i < argc; i++) {
        if (!_args[i].Evaluate(argv + i)) return false;
    }
    bool r = (*_gmlFuncs[_function])(argc, argv, NULL);
    return r ? true : !(_cause == Runtime::ReturnCause::ExitError || _cause == Runtime::ReturnCause::ExitGameEnd);
}

bool CRActionRunScript::Run() {
    GMLType argv[16];
    unsigned int argc = static_cast<unsigned int>(_args.size());
    if (argc > 16) {
        _cause = Runtime::ReturnCause::ExitError;
        _error = "Too many args to internal function, argc: " + std::to_string(argc) + ", function ID: " + std::to_string(_scriptID);
        return false;
    }
    for (unsigned int i = 0; i < argc; i++) {
        if (!_args[i].Evaluate(argv + i)) return false;
    }
    Script* scr = AssetManager::GetScript(_scriptID);
    bool r = CodeManager::Run(scr->codeObj, _context.self, _context.other, _context.eventId, _context.eventNumber, _context.objId, argc, argv);
    return r ? true : !(_cause == Runtime::ReturnCause::ExitError || _cause == Runtime::ReturnCause::ExitGameEnd);
}

bool CRActionIfElse::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;
    if (Runtime::_isTrue(&v)) {
        return _if->Run();
    }
    else if (_else) {
        return _else->Run();
    }
    return true;
}

bool CRActionWith::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;
    if (v.state != GMLTypeState::Double) {
        _cause = Runtime::ReturnCause::ExitError;
        _error = "Invalid 'with' parameter: \"" + v.sVal + "\"";
        return false;
    }
    int objID = Runtime::_round(v.dVal);

    switch (objID) {
        case SELF:
            return _code->Run();
        case OTHER: {
            Runtime::Context c = _context;
            _context.self = c.other;
            _context.other = c.self;
            _context.objId = InstanceList::GetInstance(c.other).object_index;
            bool r = _code->Run();
            c.locals = _context.locals;
            c.localInstance = _context.localInstance;
            _context = c;
            return r ? true : (_cause == Runtime::ReturnCause::Break || _cause == Runtime::ReturnCause::Continue);
        }
        case ALL: {
            Runtime::Context c = _context;
            _context.other = c.self;
            InstanceList::Iterator iter;
            InstanceHandle i;
            while ((i = iter.Next()) != InstanceList::NoInstance) {
                _context.self = i;
                _context.objId = InstanceList::GetInstance(i).object_index;
                if (!_code->Run()) {
                    if (_cause == Runtime::ReturnCause::Continue)
                        continue;
                    else {
                        c.locals = _context.locals;
                        c.localInstance = _context.localInstance;
                        _context = c;
                        return (_cause == Runtime::ReturnCause::Break);
                    }
                }
            }
            c.locals = _context.locals;
            c.localInstance = _context.localInstance;
            _context = c;
            return true;
        }
        case NOONE: {
            return true;
        }
        default: {
            if (objID < 0) {
                _cause = Runtime::ReturnCause::ExitError;
                _error = "Tried to pass negative number to 'with': " + std::to_string(objID);
                return false;
            }
            Runtime::Context c = _context;
            _context.other = c.self;
            InstanceList::Iterator iter(objID);
            InstanceHandle i;
            while ((i = iter.Next()) != InstanceList::NoInstance) {
                _context.self = i;
                _context.objId = InstanceList::GetInstance(i).object_index;
                if (!_code->Run()) {
                    if (_cause == Runtime::ReturnCause::Continue)
                        continue;
                    else {
                        c.locals = _context.locals;
                        c.localInstance = _context.localInstance;
                        _context = c;
                        return (_cause == Runtime::ReturnCause::Break);
                    }
                }
            }
            c.locals = _context.locals;
            c.localInstance = _context.localInstance;
            _context = c;
            return true;
        }
    }
}

bool CRActionRepeat::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;
    if (v.state != GMLTypeState::Double) {
        _cause = Runtime::ReturnCause::ExitError;
        _error = "Invalid repeat count: \"" + v.sVal + "\"";
        return false;
    }
    int count = Runtime::_round(v.dVal);
    for (int i = 0; i < count; i++) {
        if (!_code->Run()) {
            if (_cause == Runtime::ReturnCause::Continue)
                continue;
            else
                return (_cause == Runtime::ReturnCause::Break);
        }
    }
    return true;
}

bool CRActionWhile::Run() {
    while (true) {
        GMLType out;
        if (!_expression.Evaluate(&out)) return false;
        if (Runtime::_isTrue(&out)) {
            if (!_code->Run()) {
                if (_cause == Runtime::ReturnCause::Continue)
                    continue;
                else
                    return (_cause == Runtime::ReturnCause::Break);
            }
        }
        else {
            return true;
        }
    }
}

bool CRActionDoUntil::Run() {
    while (true) {
        if (!_code->Run()) {
            if (_cause == Runtime::ReturnCause::Continue)
                continue;
            else
                return (_cause == Runtime::ReturnCause::Break);
        }
        GMLType out;
        if (!_expression.Evaluate(&out)) return false;
        if (Runtime::_isTrue(&out)) {
            return true;
        }
    }
}

bool CRActionFor::Run() {
    if (!_initializer->Run()) return false;
    while (true) {
        GMLType out;
        if (!_check.Evaluate(&out)) return false;
        if (Runtime::_isTrue(&out)) {
            if (!_code->Run()) {
                if (_cause == Runtime::ReturnCause::Continue)
                    continue;
                else
                    return (_cause == Runtime::ReturnCause::Break);
            }
            if (!_finalizer->Run()) return false;
        }
        else {
            return true;
        }
    }
}

bool CRActionSwitch::Run() {
    GMLType exp;
    if (!_expression.Evaluate(&exp)) return false;
    unsigned int offset = _defaultOffset;
    for (SwitchCase& c : _cases) {
        GMLType caseExp;
        if (!c.expression.Evaluate(&caseExp)) return false;
        if (caseExp.state == exp.state) {
            if (exp.state == GMLTypeState::Double) {
                if (Runtime::_equal(exp.dVal, caseExp.dVal)) {
                    offset = c.offset;
                    break;
                }
            }
            else {
                if(exp.sVal == caseExp.sVal) {
                    offset = c.offset;
                    break;
                }
            }
        }
    }
    bool r = _actions.Run(offset);
    return r ? true : (_cause == Runtime::ReturnCause::Break);
}

bool CRActionBreak::Run() {
    _cause = Runtime::ReturnCause::Break;
    return false;
}

bool CRActionContinue::Run() {
    _cause = Runtime::ReturnCause::Continue;
    return false;
}

bool CRActionExit::Run() {
    _cause = Runtime::ReturnCause::ExitNormal;
    return false;
}

bool CRActionReturn::Run() {
    if (!_expression.Evaluate(&returnBuffer)) return false;
    _cause = Runtime::ReturnCause::Return;
    return false;
}

bool CRExpLiteral::_evaluate(GMLType* output) {
    (*output) = _value;
    return true;
}

bool CRExpFunction::_evaluate(GMLType* output) {
    GMLType argv[16];
    unsigned int argc = static_cast<unsigned int>(_args.size());
    for (unsigned int i = 0; i < argc; i++) {
        if (!_args[i].Evaluate(argv + i)) return false;
    }
    bool r = (*_gmlFuncs[_function])(argc, argv, output);
    return r ? true : !(_cause == Runtime::ReturnCause::ExitError || _cause == Runtime::ReturnCause::ExitGameEnd);
}

bool CRExpScript::_evaluate(GMLType* output) {
    GMLType argv[16];
    unsigned int argc = static_cast<unsigned int>(_args.size());
    for (unsigned int i = 0; i < argc; i++) {
        if (!_args[i].Evaluate(argv + i)) return false;
    }
    Script* scr = AssetManager::GetScript(_script);
    bool r = CodeManager::Run(scr->codeObj, _context.self, _context.other, _context.eventId, _context.eventNumber, _context.objId, argc, argv);
    return r ? true : !(_cause == Runtime::ReturnCause::ExitError || _cause == Runtime::ReturnCause::ExitGameEnd);
}

bool CRExpNestedExpression::_evaluate(GMLType* output) { return _expression.Evaluate(output); }

bool CRExpField::_evaluate(GMLType* output) {
    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                (*output) = *InstanceList::GetField(_context.self, _fieldNumber);
                return true;
            }
            case OTHER: {
                (*output) = *InstanceList::GetField(_context.other, _fieldNumber);
                return true;
            }
            case ALL: {
                InstanceHandle i = InstanceList::Iterator().Next();
                (*output) = *InstanceList::GetField(i, _fieldNumber);
                return true;
            }
            case NOONE: {
                (*output) = GMLType();
                return true;
            }
            case GLOBAL: {
                (*output) = _global[_fieldNumber][0];
                return true;
            }
            case LOCAL: {
                (*output) = _context.locals[_fieldNumber][0];
                return true;
            }
            default: {
                if (id < 0) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Tried to dereference negative number: " + std::to_string(id);
                    return false;
                }
                InstanceHandle i = InstanceList::Iterator(static_cast<unsigned int>(id)).Next();
                if (i != InstanceList::NoInstance) {
                    (*output) = *InstanceList::GetField(i, _fieldNumber);
                }
                else {
                    (*output) = GMLType();
                }
                return true;
            }
        }
    }
    else {
        if (_isLocal) {
            // Local
            (*output) = _context.locals[_fieldNumber][0];
            return true;
        }
        else {
            // Field
            (*output) = *InstanceList::GetField(_context.self, _fieldNumber);
            return true;
        }
    }
    return true;
}

bool CRExpArray::_evaluate(GMLType* output) {
    int index = 0;
    if (!_evalArrayAccessor(_dimensions, &index)) return false;

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                (*output) = *InstanceList::GetField(_context.self, _fieldNumber, index);
                return true;
            }
            case OTHER: {
                (*output) = *InstanceList::GetField(_context.other, _fieldNumber, index);
                return true;
            }
            case ALL: {
                InstanceHandle i = InstanceList::Iterator().Next();
                if (i) {
                    (*output) = *InstanceList::GetField(i, _fieldNumber, index);
                }
                else {
                    (*output) = GMLType();
                }
                return true;
            }
            case NOONE: {
                (*output) = GMLType();
                return true;
            }
            case GLOBAL: {
                (*output) = _global[_fieldNumber][index];
                return true;
            }
            case LOCAL: {
                (*output) = _context.locals[_fieldNumber][index];
                return true;
            }
            default: {
                if (id < 0) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Tried to dereference negative number: " + std::to_string(id);
                    return false;
                }
                InstanceHandle i = InstanceList::Iterator(static_cast<unsigned int>(id)).Next();
                if (i != InstanceList::NoInstance) {
                    (*output) = *InstanceList::GetField(i, _fieldNumber, index);
                }
                else {
                    (*output) = GMLType();
                }
                return true;
            }
        }
    }
    else {
        if (_isLocal) {
            // Local
            (*output) = _context.locals[_fieldNumber][index];
            return true;
        }
        else {
            // Field
            (*output) = *InstanceList::GetField(_context.self, _fieldNumber, index);
            return true;
        }
    }
    return true;
}

bool CRExpInstanceVar::_evaluate(GMLType* output) {
    int index = 0;
    if (!_evalArrayAccessor(_dimensions, &index)) return false;

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                return _getInstanceVar(InstanceList::GetInstance(_context.self), _var, index, output);
            }
            case OTHER: {
                return _getInstanceVar(InstanceList::GetInstance(_context.other), _var, index, output);
            }
            case ALL: {
                InstanceHandle i = InstanceList::Iterator().Next();
                if (i) {
                    return _getInstanceVar(InstanceList::GetInstance(i), _var, index, output);
                }
                else {
                    (*output) = GMLType();
                    return true;
                }
            }
            case GLOBAL: {
                (*output) = _globalInstance[_var][index];
                return true;
            }
            case LOCAL: {
                (*output) = _context.localInstance[_var][index];
                return true;
            }
            default: {
                if (id < 0) {
                    _cause = Runtime::ReturnCause::ExitError;
                    _error = "Tried to dereference negative number: " + std::to_string(id);
                    return false;
                }
                InstanceHandle i = InstanceList::Iterator(static_cast<unsigned int>(id)).Next();
                if (i != InstanceList::NoInstance) {
                    return _getInstanceVar(InstanceList::GetInstance(i), _var, index, output);
                }
                else {
                    (*output) = GMLType();
                    return true;
                }
            }
        }
    }
    else {
        return _getInstanceVar(InstanceList::GetInstance(_context.self), _var, index, output);
    }
    return true;
}

bool CRExpGameVar::_evaluate(GMLType* output) {
    int index = 0;
    if (!_evalArrayAccessor(_dimensions, &index)) return false;
    return _getGameValue(_var, index, output);
}