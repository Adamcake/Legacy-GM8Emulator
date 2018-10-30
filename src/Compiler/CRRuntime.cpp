#include <pch.h>

#include "Alarm.hpp"
#include "AssetManager.hpp"
#include "CRRuntime.hpp"
#include "Collision.hpp"
#include "Compiled.hpp"
#include "GlobalValues.hpp"
#include "Instance.hpp"
#include "InstanceList.hpp"
#include "Renderer.hpp"

GlobalValues* _globalValues;
std::map<unsigned int, std::map<unsigned int, GMLType>> _global;
std::vector<bool (*)(unsigned int, GMLType*, GMLType*)> _gmlFuncs;

void Runtime::Init(GlobalValues* globals, std::vector<bool (*)(unsigned int, GMLType*, GMLType*)>& gmlFuncs) {
    _globalValues = globals;
    _gmlFuncs = gmlFuncs;
}

void Runtime::Finalize() {}

GlobalValues* Runtime::GetGlobals() { return _globalValues; }


Runtime::Context _context;
Runtime::Context Runtime::GetContext() { return _context; }


bool CRExpressionValue::Evaluate(GMLType* output) {
    if(!this->_evaluate(output)) return false;
    for(CRUnaryOperator op : _unary) {
        if(output->state == GMLTypeState::String) return false;
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
        case INSTANCE_COUNT:
            out->dVal = ( double )InstanceList::Count();
            break;
        case MOUSE_X:
            int mx;
            RGetCursorPos(&mx, NULL);
            out->dVal = ( double )mx;
            break;
        case MOUSE_Y:
            int my;
            RGetCursorPos(NULL, &my);
            out->dVal = ( double )my;
            break;
        case ROOM:
            out->dVal = ( double )_globalValues->room;
            break;
        case ROOM_SPEED:
            out->dVal = ( double )_globalValues->room_speed;
            break;
        case ROOM_WIDTH:
            out->dVal = ( double )_globalValues->room_width;
            break;
        case ROOM_HEIGHT:
            out->dVal = ( double )_globalValues->room_height;
            break;
        default:
            return false;
    }
    return true;
}


bool _setInstanceVar(Instance* instance, CRInstanceVar index, unsigned int arrayIndex, CRSetMethod method, GMLType value) {
    // No instance vars are strings. In GML if you set an instance var to a string, it gets set to 0.
    GMLType t;
    t.state = GMLTypeState::Double;
    if (value.state != GMLTypeState::Double) {
        value.state = GMLTypeState::Double;
        value.dVal = 0;
    }

    switch (index) {
        case IV_ALARM: {
            int alarmValue = ( int )(value.state == GMLTypeState::Double ? value.dVal : 0.0);
            if (alarmValue)
                AlarmSet(instance->id, ( unsigned int )arrayIndex, alarmValue);
            else
                AlarmDelete(instance->id, ( unsigned int )arrayIndex);
            break;
        }
        case IV_DIRECTION:
            t.dVal = instance->direction;
            if (!_applySetMethod(&t, method, &value)) return false;
            while (t.dVal >= 360.0) t.dVal -= 360.0;
            while (t.dVal < 0.0) t.dVal += 360.0;
            instance->direction = (t.dVal);
            instance->hspeed = ::cos(instance->direction * GML_PI / 180.0) * instance->speed;
            instance->vspeed = -::sin(instance->direction * GML_PI / 180.0) * instance->speed;
            break;
        case IV_IMAGE_SPEED:
            t.dVal = instance->image_speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->image_speed = t.dVal;
            break;
        case IV_FRICTION:
            t.dVal = instance->friction;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->friction = t.dVal;
            break;
        case IV_SPRITE_INDEX:
            t.dVal = instance->sprite_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->sprite_index = Runtime::_round(t.dVal);
            break;
        case IV_MASK_INDEX:
            t.dVal = instance->mask_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->mask_index = Runtime::_round(t.dVal);
            break;
        case IV_IMAGE_BLEND:
            t.dVal = instance->image_blend;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->image_blend = Runtime::_round(t.dVal);
            break;
        case IV_IMAGE_ALPHA:
            t.dVal = instance->image_alpha;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->image_alpha = t.dVal;
            break;
        case IV_IMAGE_INDEX:
            t.dVal = instance->image_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->image_index = t.dVal;
            break;
        case IV_IMAGE_ANGLE:
            t.dVal = instance->image_angle;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->image_angle = t.dVal;
            instance->bboxIsStale = true;
            break;
        case IV_IMAGE_XSCALE:
            t.dVal = instance->image_xscale;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->image_xscale = t.dVal;
            instance->bboxIsStale = true;
            break;
        case IV_IMAGE_YSCALE:
            t.dVal = instance->image_yscale;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->image_yscale = t.dVal;
            instance->bboxIsStale = true;
            break;
        case IV_SOLID:
            t.dVal = (instance->solid ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->solid = Runtime::_isTrue(&t);
            break;
        case IV_VISIBLE:
            t.dVal = (instance->visible ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->visible = Runtime::_isTrue(&t);
            break;
        case IV_PERSISTENT:
            t.dVal = (instance->persistent ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->persistent = Runtime::_isTrue(&t);
            break;
        case IV_DEPTH:
            t.dVal = instance->depth;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->depth = Runtime::_round(t.dVal);
            break;
        case IV_SPEED:
            t.dVal = instance->speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->speed = t.dVal;
            instance->hspeed = ::cos(instance->direction * GML_PI / 180.0) * instance->speed;
            instance->vspeed = -::sin(instance->direction * GML_PI / 180.0) * instance->speed;
            break;
        case IV_VSPEED:
            t.dVal = instance->vspeed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->vspeed = t.dVal;
            instance->direction = (instance->hspeed == 0) ? ::abs(instance->vspeed) : (::atan(-instance->vspeed / instance->hspeed) * 180.0 / GML_PI);
            instance->speed = ::sqrt(pow(instance->hspeed, 2) + pow(instance->vspeed, 2));
            break;
        case IV_HSPEED:
            t.dVal = instance->hspeed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->hspeed = t.dVal;
            instance->direction = (instance->hspeed == 0) ? ::abs(instance->vspeed) : (::atan(-instance->vspeed / instance->hspeed) * 180.0 / GML_PI);
            instance->speed = ::sqrt(pow(instance->hspeed, 2) + pow(instance->vspeed, 2));
            break;
        case IV_GRAVITY:
            t.dVal = instance->gravity;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->gravity = t.dVal;
            break;
        case IV_GRAVITY_DIRECTION:
            t.dVal = instance->gravity_direction;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->gravity_direction = t.dVal;
            break;
        case IV_X:
            t.dVal = instance->x;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->x = t.dVal;
            instance->bboxIsStale = true;
            break;
        case IV_Y:
            t.dVal = instance->y;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->y = t.dVal;
            instance->bboxIsStale = true;
            break;
        case IV_PATH_INDEX:
            t.dVal = instance->path_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->path_index = Runtime::_round(t.dVal);
            break;
        case IV_PATH_POSITION:
            t.dVal = instance->path_position;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->path_position = t.dVal;
            break;
        case IV_PATH_SPEED:
            t.dVal = instance->path_speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->path_speed = t.dVal;
            break;
        case IV_PATH_SCALE:
            t.dVal = instance->path_scale;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->path_scale = t.dVal;
            break;
        case IV_PATH_ORIENTATION:
            t.dVal = instance->path_orientation;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->path_orientation = t.dVal;
            break;
        case IV_PATH_ENDACTION:
            t.dVal = instance->path_endaction;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->path_endaction = Runtime::_round(t.dVal);
            break;
        case IV_TIMELINE_INDEX:
            t.dVal = instance->timeline_index;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->timeline_index = Runtime::_round(t.dVal);
            break;
        case IV_TIMELINE_RUNNING:
            t.dVal = (instance->timeline_running ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->timeline_running = Runtime::_isTrue(&t);
            break;
        case IV_TIMELINE_LOOP:
            t.dVal = (instance->timeline_loop ? GMLTrue : GMLFalse);
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->timeline_loop = Runtime::_isTrue(&t);
            break;
        case IV_TIMELINE_SPEED:
            t.dVal = instance->timeline_speed;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->timeline_speed = t.dVal;
            break;
        case IV_TIMELINE_POSITION:
            t.dVal = instance->timeline_position;
            if (!_applySetMethod(&t, method, &value)) return false;
            instance->timeline_position = t.dVal;
            break;
        default:
            return false;
    }
    return true;
}

bool _getInstanceVar(Instance* instance, CRInstanceVar index, unsigned int arrayIndex, GMLType* out) {
    out->state = GMLTypeState::Double;
    switch (index) {
        case IV_ALARM: {
            out->dVal = ( double )AlarmGet(instance->id, arrayIndex);
            break;
        }
        case IV_INSTANCE_ID:
            out->dVal = instance->id;
            break;
        case IV_X:
            out->dVal = instance->x;
            break;
        case IV_Y:
            out->dVal = instance->y;
            break;
        case IV_XPREVIOUS:
            out->dVal = instance->xprevious;
            break;
        case IV_YPREVIOUS:
            out->dVal = instance->yprevious;
            break;
        case IV_XSTART:
            out->dVal = instance->xstart;
            break;
        case IV_YSTART:
            out->dVal = instance->ystart;
            break;
        case IV_SOLID:
            out->dVal = (instance->solid ? GMLTrue : GMLFalse);
            break;
        case IV_DEPTH:
            out->dVal = ( double )instance->depth;
            break;
        case IV_VISIBLE:
            out->dVal = (instance->visible ? GMLTrue : GMLFalse);
            break;
        case IV_PERSISTENT:
            out->dVal = (instance->persistent ? GMLTrue : GMLFalse);
            break;
        case IV_DIRECTION:
            out->dVal = instance->direction;
            break;
        case IV_SPEED:
            out->dVal = instance->speed;
            break;
        case IV_VSPEED:
            out->dVal = instance->vspeed;
            break;
        case IV_HSPEED:
            out->dVal = instance->hspeed;
            break;
        case IV_GRAVITY:
            out->dVal = instance->gravity;
            break;
        case IV_GRAVITY_DIRECTION:
            out->dVal = instance->gravity_direction;
            break;
        case IV_FRICTION:
            out->dVal = instance->friction;
            break;
        case IV_IMAGE_INDEX:
            out->dVal = instance->image_index;
            break;
        case IV_IMAGE_SPEED:
            out->dVal = instance->image_speed;
            break;
        case IV_SPRITE_INDEX:
            out->dVal = instance->sprite_index;
            break;
        case IV_SPRITE_WIDTH:
            if (instance->sprite_index < 0 || instance->sprite_index >= ( int )AssetManager::GetSpriteCount()) {
                out->dVal = 0;
            }
            else {
                Sprite* s = AssetManager::GetSprite(instance->sprite_index);
                out->dVal = (s->exists ? s->width : 0);
            }
            break;
        case IV_SPRITE_HEIGHT:
            if (instance->sprite_index < 0 || instance->sprite_index >= ( int )AssetManager::GetSpriteCount()) {
                out->dVal = 0;
            }
            else {
                Sprite* s = AssetManager::GetSprite(instance->sprite_index);
                out->dVal = (s->exists ? s->height : 0);
            }
            break;
        case IV_MASK_INDEX:
            out->dVal = instance->mask_index;
            break;
        case IV_IMAGE_XSCALE:
            out->dVal = instance->image_xscale;
            break;
        case IV_IMAGE_YSCALE:
            out->dVal = instance->image_yscale;
            break;
        case IV_IMAGE_ANGLE:
            out->dVal = instance->image_angle;
            break;
        case IV_IMAGE_BLEND:
            out->dVal = ( double )instance->image_blend;
            break;
        case IV_IMAGE_ALPHA:
            out->dVal = instance->image_alpha;
            break;
        case IV_PATH_INDEX:
            out->dVal = ( double )instance->path_index;
            break;
        case IV_PATH_POSITION:
            out->dVal = instance->path_position;
            break;
        case IV_PATH_SPEED:
            out->dVal = instance->path_speed;
            break;
        case IV_PATH_SCALE:
            out->dVal = instance->path_scale;
            break;
        case IV_PATH_ORIENTATION:
            out->dVal = instance->path_orientation;
            break;
        case IV_PATH_ENDACTION:
            out->dVal = instance->path_endaction;
            break;
        case IV_PATH_POSITIONPREVIOUS:
            out->dVal = instance->path_positionprevious;
            break;
        case IV_TIMELINE_INDEX:
            out->dVal = ( double )instance->timeline_index;
            break;
        case IV_TIMELINE_RUNNING:
            out->dVal = instance->timeline_running ? GMLTrue : GMLFalse;
            break;
        case IV_TIMELINE_LOOP:
            out->dVal = instance->timeline_loop ? GMLTrue : GMLFalse;
            break;
        case IV_TIMELINE_SPEED:
            out->dVal = instance->timeline_speed;
            break;
        case IV_TIMELINE_POSITION:
            out->dVal = instance->timeline_position;
            break;
        case IV_BBOX_LEFT:
            RefreshInstanceBbox(instance);
            out->dVal = instance->bbox_left;
            break;
        case IV_BBOX_RIGHT:
            RefreshInstanceBbox(instance);
            out->dVal = instance->bbox_right;
            break;
        case IV_BBOX_BOTTOM:
            RefreshInstanceBbox(instance);
            out->dVal = instance->bbox_bottom;
            break;
        case IV_BBOX_TOP:
            RefreshInstanceBbox(instance);
            out->dVal = instance->bbox_top;
            break;
        default:
            return false;
    }
    return true;
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

bool Runtime::_isTrue(const GMLType* value) { return (value->state == GMLTypeState::Double) && (value->dVal >= 0.5); }


bool Runtime::Execute(CRActionList& actions, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, unsigned int argc, GMLType* argv) {
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
    return result;
}

bool Runtime::EvalExpression(CRExpression& expression, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, GMLType* out) {
    Context c = _context;
    _context.self = self;
    _context.other = other;
    _context.eventId = ev;
    _context.eventNumber = sub;
    _context.objId = asObjId;
    _context.argc = 0;
    _context.argv = nullptr;
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


bool CRActionBindVars::Run() {
    for (unsigned int field : _fields) {
        _context.locals[field][0] = GMLType();
    }
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
                if (!_applySetMethod(InstanceList::GetField(_context.self->id, _field), _method, &v)) return false;
            }
            case OTHER: {
                if (!_applySetMethod(InstanceList::GetField(_context.other->id, _field), _method, &v)) return false;
            }
            case ALL: {
                InstanceList::Iterator iter;
                Instance* i;
                while (i = iter.Next()) {
                    if (!_applySetMethod(InstanceList::GetField(i->id, _field), _method, &v)) return false;
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
                if (id < 0) return false;
                InstanceList::Iterator iter(( unsigned int )id);
                Instance* i;
                while (i = iter.Next()) {
                    if (!_applySetMethod(InstanceList::GetField(i->id, _field), _method, &v)) return false;
                }
            }
        }
    }
    else {
        if (_context.locals.count(_field)) {
            // Local
            if (!_applySetMethod(&_context.locals[_field][0], _method, &v)) return false;
        }
        else {
            // Field
            if (!_applySetMethod(InstanceList::GetField(_context.self->id, _field), _method, &v)) return false;
        }
    }
    return true;
}

bool CRActionAssignmentArray::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;

    int index = 0;
    if (_dimensions.size()) {
        if (_dimensions.size() > 2) return false;
        GMLType dim1;
        if (!_dimensions[0].Evaluate(&dim1)) return false;
        if (dim1.state == GMLTypeState::String) return false;
        index = Runtime::_round(dim1.dVal);
        if (index < 0) return false;

        if (_dimensions.size() == 2) {
            GMLType dim2;
            if (!_dimensions[1].Evaluate(&dim2)) return false;
            if (dim2.state == GMLTypeState::String) return false;
            int id2 = Runtime::_round(dim2.dVal);
            if (id2 < 0) return false;
            index = (index * 32000) + id2;
        }
    }

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                if (!_applySetMethod(InstanceList::GetField(_context.self->id, _field, index), _method, &v)) return false;
                break;
            }
            case OTHER: {
                if (!_applySetMethod(InstanceList::GetField(_context.other->id, _field, index), _method, &v)) return false;
                break;
            }
            case ALL: {
                InstanceList::Iterator iter;
                Instance* i;
                while (i = iter.Next()) {
                    if (!_applySetMethod(InstanceList::GetField(i->id, _field, index), _method, &v)) return false;
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
                if (id < 0) return false;
                InstanceList::Iterator iter(( unsigned int )id);
                Instance* i;
                while (i = iter.Next()) {
                    if (!_applySetMethod(InstanceList::GetField(i->id, _field, index), _method, &v)) return false;
                }
            }
        }
    }
    else {
        if (_context.locals.count(_field)) {
            // Local
            if (!_applySetMethod(&_context.locals[_field][index], _method, &v)) return false;
        }
        else {
            // Field
            if (!_applySetMethod(InstanceList::GetField(_context.self->id, _field, index), _method, &v)) return false;
        }
    }
    return true;
}

bool CRActionAssignmentInstanceVar::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;

    int index = 0;
    if (_dimensions.size()) {
        if (_dimensions.size() > 2) return false;
        GMLType dim1;
        if (!_dimensions[0].Evaluate(&dim1)) return false;
        if (dim1.state == GMLTypeState::String) return false;
        index = Runtime::_round(dim1.dVal);
        if (index < 0) return false;

        if (_dimensions.size() == 2) {
            GMLType dim2;
            if (!_dimensions[1].Evaluate(&dim2)) return false;
            if (dim2.state == GMLTypeState::String) return false;
            int id2 = Runtime::_round(dim2.dVal);
            if (id2 < 0) return false;
            index = (index * 32000) + id2;
        }
    }

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                if (!_setInstanceVar(_context.self, _var, index, _method, v)) return false;
                break;
            }
            case OTHER: {
                if (!_setInstanceVar(_context.other, _var, index, _method, v)) return false;
                break;
            }
            case ALL: {
                InstanceList::Iterator iter;
                Instance* i;
                while (i = iter.Next()) {
                    if (!_setInstanceVar(i, _var, index, _method, v)) return false;
                }
                break;
            }
            case GLOBAL: {
                // uh
                return false;
            }
            case LOCAL: {
                // uh
                return false;
            }
            default: {
                if (id < 0) return false;
                InstanceList::Iterator iter(( unsigned int )id);
                Instance* i;
                while (i = iter.Next()) {
                    if (!_setInstanceVar(i, _var, index, _method, v)) return false;
                }
            }
        }
    }
    else {
        if (!_setInstanceVar(_context.self, _var, index, _method, v)) return false;
    }
    return true;
}

bool CRActionAssignmentGameVar::Run() { return false; }

bool CRActionBlock::Run() { return false; }

bool CRActionRunFunction::Run() {
    GMLType argv[16];
    unsigned int argc = static_cast<unsigned int>(_args.size());
    for (unsigned int i = 0; i < argc; i++) {
        if (!_args[i].Evaluate(argv + i)) return false;
    }
    return (*_gmlFuncs[_function])(argc, argv, NULL);
}

bool CRActionRunScript::Run() { return false; }

bool CRActionIfElse::Run() {
    GMLType v;
    if (!_expression.Evaluate(&v)) return false;
    if(Runtime::_isTrue(&v)) {
        return _if->Run();
    }
    else if(_else) {
        return _else->Run();
    }
    return true;
}

bool CRActionWith::Run() { return false; }

bool CRActionSwitch::Run() { return false; }

bool CRActionBreak::Run() { return false; }

bool CRActionContinue::Run() { return false; }

bool CRActionExit::Run() { return false; }

bool CRActionReturn::Run() { return false; }

bool CRExpLiteral::_evaluate(GMLType* output) {
    (*output) = _value;
    return true;
}

bool CRExpFunction::_evaluate(GMLType* output) {
    GMLType argv[16];
    unsigned int argc = _args.size();
    for (unsigned int i = 0; i < argc; i++) {
        if (!_args[i].Evaluate(argv + i)) return false;
    }
    return (*_gmlFuncs[_function])(argc, argv, output);
}

bool CRExpScript::_evaluate(GMLType* output) { return false; }

bool CRExpNestedExpression::_evaluate(GMLType* output) {
    return _expression.Evaluate(output);
}

bool CRExpField::_evaluate(GMLType* output) {
    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                (*output) = *InstanceList::GetField(_context.self->id, _fieldNumber);
                return true;
            }
            case OTHER: {
                (*output) = *InstanceList::GetField(_context.other->id, _fieldNumber);
                return true;
            }
            case ALL: {
                Instance* i = InstanceList::Iterator().Next();
                (*output) = *InstanceList::GetField(i->id, _fieldNumber);
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
                if (id < 0) return false;
                Instance* i = InstanceList::Iterator(( unsigned int )id).Next();
                (*output) = *InstanceList::GetField(i->id, _fieldNumber);
                return true;
            }
        }
    }
    else {
        if (_context.locals.count(_fieldNumber)) {
            // Local
            (*output) = _context.locals[_fieldNumber][0];
            return true;
        }
        else {
            // Field
            (*output) = *InstanceList::GetField(_context.self->id, _fieldNumber);
            return true;
        }
    }
    return true;
}

bool CRExpArray::_evaluate(GMLType* output) {
    int index = 0;
    if (_dimensions.size()) {
        if (_dimensions.size() > 2) return false;
        GMLType dim1;
        if (!_dimensions[0].Evaluate(&dim1)) return false;
        if (dim1.state == GMLTypeState::String) return false;
        index = Runtime::_round(dim1.dVal);
        if (index < 0) return false;

        if (_dimensions.size() == 2) {
            GMLType dim2;
            if (!_dimensions[1].Evaluate(&dim2)) return false;
            if (dim2.state == GMLTypeState::String) return false;
            int id2 = Runtime::_round(dim2.dVal);
            if (id2 < 0) return false;
            index = (index * 32000) + id2;
        }
    }

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                (*output) = *InstanceList::GetField(_context.self->id, _fieldNumber, index);
                return true;
            }
            case OTHER: {
                (*output) = *InstanceList::GetField(_context.other->id, _fieldNumber, index);
                return true;
            }
            case ALL: {
                Instance* i = InstanceList::Iterator().Next();
                if (i) {
                    (*output) = *InstanceList::GetField(i->id, _fieldNumber, index);
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
                if (id < 0) return false;
                Instance* i = InstanceList::Iterator(( unsigned int )id).Next();
                if (i) {
                    (*output) = *InstanceList::GetField(i->id, _fieldNumber, index);
                }
                else {
                    (*output) = GMLType();
                }
                return true;
            }
        }
    }
    else {
        if (_context.locals.count(_fieldNumber)) {
            // Local
            (*output) = _context.locals[_fieldNumber][index];
            return true;
        }
        else {
            // Field
            (*output) = *InstanceList::GetField(_context.self->id, _fieldNumber, index);
            return true;
        }
    }
    return true;
}

bool CRExpInstanceVar::_evaluate(GMLType* output) {
    int index = 0;
    if (_dimensions.size()) {
        if (_dimensions.size() > 2) return false;
        GMLType dim1;
        if (!_dimensions[0].Evaluate(&dim1)) return false;
        if (dim1.state == GMLTypeState::String) return false;
        index = Runtime::_round(dim1.dVal);
        if (index < 0) return false;

        if (_dimensions.size() == 2) {
            GMLType dim2;
            if (!_dimensions[1].Evaluate(&dim2)) return false;
            if (dim2.state == GMLTypeState::String) return false;
            int id2 = Runtime::_round(dim2.dVal);
            if (id2 < 0) return false;
            index = (index * 32000) + id2;
        }
    }

    if (_hasDeref) {
        GMLType d;
        if (!_deref.Evaluate(&d)) return false;

        int id = Runtime::_round(d.dVal);
        switch (id) {
            case SELF: {
                return _getInstanceVar(_context.self, _var, index, output);
            }
            case OTHER: {
                return _getInstanceVar(_context.other, _var, index, output);
            }
            case ALL: {
                Instance* i = InstanceList::Iterator().Next();
                if (i) {
                    return _getInstanceVar(i, _var, index, output);
                }
                else {
                    (*output) = GMLType();
                    return true;
                }
            }
            case GLOBAL: {
                // uh
                return false;
            }
            case LOCAL: {
                // uh
                return false;
            }
            default: {
                if (id < 0) return false;
                Instance* i = InstanceList::Iterator(( unsigned int )id).Next();
                if (i) {
                    return _getInstanceVar(i, _var, index, output);
                }
                else {
                    (*output) = GMLType();
                    return true;
                }
            }
        }
    }
    else {
        return _getInstanceVar(_context.self, _var, index, output);
    }
    return true;
}

bool CRExpGameVar::_evaluate(GMLType* output) {
    int index = 0;
    if (_dimensions.size()) {
        if (_dimensions.size() > 2) return false;
        GMLType dim1;
        if (!_dimensions[0].Evaluate(&dim1)) return false;
        if (dim1.state == GMLTypeState::String) return false;
        index = Runtime::_round(dim1.dVal);
        if (index < 0) return false;

        if (_dimensions.size() == 2) {
            GMLType dim2;
            if (!_dimensions[1].Evaluate(&dim2)) return false;
            if (dim2.state == GMLTypeState::String) return false;
            int id2 = Runtime::_round(dim2.dVal);
            if (id2 < 0) return false;
            index = (index * 32000) + id2;
        }
    }

    return _getGameValue(_var, index, output);
}