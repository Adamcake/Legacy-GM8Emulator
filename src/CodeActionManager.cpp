#include <pch.h>

#include "CodeActionManager.hpp"
#include "AssetManager.hpp"
#include "CRGMLType.hpp"
#include "CodeRunner.hpp"
#include "Instance.hpp"
#include "InstanceList.hpp"
#include "StreamUtil.hpp"

// Private vars
namespace CodeActionManager {
    class Parameter {
      public:
        virtual bool Evaluate(Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, GMLType* out) = 0;
        virtual bool Compile() { return true; }
        virtual ~Parameter() {}
    };

    class ParamExpression : public Parameter {
      private:
        CodeObject _exp;

      public:
        ParamExpression(CodeObject exp) : _exp(exp) {}
        ~ParamExpression() {}
        virtual bool Evaluate(Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, GMLType* out) { return CodeManager::Query(_exp, self, other, ev, sub, asObjId, out); }
        virtual bool Compile() override { return CodeManager::Compile(_exp); }
    };

    class ParamGML : public Parameter {
      private:
        CodeObject _code;

      public:
        ParamGML(CodeObject code) : _code(code) {}
        ~ParamGML() {}
        virtual bool Evaluate(Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, GMLType* out) { return CodeManager::Run(_code, self, other, ev, sub, asObjId); }
        virtual bool Compile() override { return CodeManager::Compile(_code); }
    };

    class ParamLiteral : public Parameter {
      private:
        GMLType _param;

      public:
        ParamLiteral(int p) { _param.dVal = static_cast<double>(p); }
        ParamLiteral(const char* p) { _param.sVal = p; }
        ~ParamLiteral() {}
        virtual bool Evaluate(Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, GMLType* out) {
            (*out) = _param;
            return true;
        }
    };

    struct CACodeAction {
        unsigned int actionID;
        Parameter* params[8];
        unsigned int paramCount;
        CodeObject codeObj;
        bool question;
        bool appliesToSomething;
        int appliesTo;
        unsigned int param;  // Currently only used for repeat blocks because the game object needs to know how many times to repeat.
    };
    std::vector<CACodeAction> _actions;
}

bool CodeActionManager::Init() {
    return true;
}

void CodeActionManager::Finalize() {
    for(CACodeAction& action : _actions) {
        for(unsigned int i = 0; i < action.paramCount; i++) {
            delete action.params[i];
        }
    }
}

bool CodeActionManager::Read(const unsigned char* stream, unsigned int* pos, CodeAction* out) {
    CACodeAction action;

    (*pos) += 8;  // Skips version id and useless lib id
    action.actionID = ReadDword(stream, pos);
    (*pos) += 8;  // Skips the useless "kind" variable and a flag for whether it can be relative
    action.question = ReadDword(stream, pos);
    action.appliesToSomething = ReadDword(stream, pos);
    (*pos) += 4;                       // Skips the useless "type" var
    (*pos) += ReadDword(stream, pos);  // Function name?
    (*pos) += ReadDword(stream, pos);  // Function code?

    action.paramCount = ReadDword(stream, pos);
    if (action.paramCount > 8)
        return false;  // There's space for 8 args per action. If we try to read more than this, something's wrong.

    (*pos) += 4;  // Version id
    unsigned int types[8];
    unsigned int i;
    for (i = 0; i < 8; i++) {
        types[i] = ReadDword(stream, pos);
    }

    action.appliesTo = static_cast<int>(ReadDword(stream, pos));
    bool relative = ReadDword(stream, pos);

    (*pos) += 4;  // Another version id...

    char** args = new char*[action.paramCount];
    unsigned int lengths[8];
    for (i = 0; i < action.paramCount; i++) {
        args[i] = ReadString(stream, pos, lengths + i);
    }

    (*pos) += ((8 - i) * 5);  // Skip unused arg strings. These should all be 1-length strings that say "0".
    bool _not = ReadDword(stream, pos);

    for (i = 0; i < action.paramCount; i++) {
        switch (types[i]) {
            case 0:  // expression
                action.params[i] = new ParamExpression(CodeManager::RegisterQuestion(args[i], lengths[i]));
                break;
            case 1:  // gml
                action.params[i] = new ParamGML(CodeManager::Register(args[i], lengths[i]));
                break;
            case 2:
                action.params[i] = new ParamLiteral(args[i]);
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
                action.params[i] = new ParamLiteral(std::atoi(args[i]));
                break;
            default:
                char* c = args[i];
                return false;
        }
    }

    // Now we have to generate some GML and register this with the code runner.
    std::string gml;
    switch (action.actionID) {
        case 101: {
            double direction = 0.0;
            unsigned int dirCount = 0;
            if (args[0][0] == '1') {
                direction += 135;
                dirCount++;
            }
            if (args[0][1] == '1') {
                direction += 90;
                dirCount++;
            }
            if (args[0][2] == '1') {
                direction += 45;
                dirCount++;
            }
            if (args[0][3] == '1') {
                direction += 180;
                dirCount++;
            }
            if (args[0][5] == '1') {
                dirCount++;
            }
            if (args[0][6] == '1') {
                direction += 225;
                dirCount++;
            }
            if (args[0][7] == '1') {
                direction += 270;
                dirCount++;
            }
            if (args[0][8] == '1') {
                direction += 315;
                dirCount++;
            }
            if (dirCount) direction /= dirCount;
            if (relative)
                gml = "speed+=argument[1];direction=";
            else
                gml = "speed=argument[1];direction=";
            gml += std::to_string(direction);
            break;
        }
        case 102:
            // Start moving in a direction
            if (relative)
                gml = "direction=argument[0];speed+=argument[1];";
            else
                gml = "direction=argument[0];speed=argument[1];";
            break;
        case 103:
            // Set the horizontal speed
            if (relative)
                gml = "hspeed+=argument[0]";
            else
                gml = "hspeed=argument[0]";
            break;
        case 104:
            if (relative)
                gml = "vspeed+=argument[0]";
            else
                gml = "vspeed=argument[0]";
            // Set the vertical speed
            break;
        case 105:
            // Move towards point
            if (relative)
                gml = "move_towards_point(x+argument[0],y+argument[1],argument[2])";
            else
                gml = "move_towards_point(argument[0],argument[1],argument[2])";
            break;
        case 107: {
            // Set gravity
            if (relative)
                gml = "gravity+=argument[1];gravity_direction=argument[0];";
            else
                gml = "gravity=argument[1];gravity_direction=argument[0];";
            break;
        }
        case 108: {
            // Set friction
            if (relative)
                gml = "friction+=argument[0]";
            else
                gml = "friction=argument[0]";
            break;
        }
        case 109: {
            // Jump to position
            if (relative)
                gml = "x+=argument[1];y+=argument[0];";
            else
                gml = "x=argument[1];y=argument[0];";
            break;
        }
        case 110: {
            // Jump to start
            gml = "x=xstart;y=ystart;";
            break;
        }
        case 111: {
            // Jump to random
            gml = "move_random(argument[0],argument[1])";
            break;
        }
        case 112: {
            // Wrap around screen - Note: I think this is right. It's been accurate in every case I've tested so far.
            const char* baseHorGml = "if((hspeed>0)&&(x>=room_width))x-=(room_width+sprite_width)if((hspeed<0)&&(x<0))x+=(room_width+sprite_width)";
            const char* baseVertGml = "if((vspeed>0)&&(y>=room_height))y-=(room_height+sprite_height)if((vspeed<0)&&(y<0))y+=(room_height+sprite_height)";
            bool hor = true;
            bool vert = true;
            if (args[0][0] == '1')
                hor = false;
            else if (args[0][0] == '0')
                vert = false;
            if (hor) gml += baseHorGml;
            if (vert) gml += baseVertGml;
            break;
        }
        case 113: {
            // reverse horizontally
            gml = "hspeed=-hspeed";
            break;
        }
        case 114: {
            // reverse vertically
            gml = "vspeed=-vspeed";
            break;
        }
        case 115: {
            // Move bounce
            gml = (args[1][0] == '0') ? "move_bounce_solid(argument[0])" : "move_bounce_all(argument[0])";
            break;
        }
        case 119: {
            // Set path
            gml = "path_start(argument[0],argument[1],argument[2],argument[3])";
            break;
        }
        case 124: {
            gml = "path_end()";
            break;
        }
        case 201: {
            // Create an instance
            if (relative)
                gml = "instance_create(x+argument[1],y+argument[2],argument[0])";
            else
                gml = "instance_create(argument[1],argument[2],argument[0])";
            break;
        }
        case 202: {
            // Change the instance
            gml = "instance_change(argument[0],argument[1])";
            break;
        }
        case 203: {
            // Destroy the instance
            gml = "instance_destroy()";
            break;
        }
        case 204: {
            // Destroy instances at position
            if (relative)
                gml = "position_destroy(argument[0]+x,argument[1]+y)";
            else
                gml = "position_destroy(argument[0],argument[1])";
            break;
        }
        case 206: {
            // Create a moving instance
            if (relative)
                gml = "var a;a=instance_create(x+argument[1],y+argument[2],argument[0]);a.speed=argument[3];a.direction=argument[4];";
            else
                gml = "var a;a=instance_create(argument[1],argument[2],argument[0]);a.speed=argument[3];a.direction=argument[4];";
            break;
        }
        case 211: {
            gml = "sound_play(argument[0])";
            break;
        }
        case 212: {
            gml = "sound_stop(argument[0])";
        }
        case 222: {
            // Go to next room
            gml = "transition_kind=argument[0];room_goto_next()";
            break;
        }
        case 224: {
            // Go to room
            gml = "transition_kind=argument[1];room_goto(argument[0])";
            break;
        }
        case 301: {
            // Set alarm
            if (relative)
                gml = "alarm[argument[1]]+=argument[0]";
            else
                gml = "alarm[argument[1]]=argument[0]";
            break;
        }
        case 302: {
            // Sleep
            gml = "sleep(argument[0])";
            break;
        }
        case 304: {
            // Set timeline position
            gml = "timeline_position=argument[0];";
            break;
        }
        case 305: {
            // Set timeline
            gml = "timeline_index=argument[0];timeline_position=argument[1];timeline_running=!argument[2];timeline_loop=argument[3];";
            break;
        }
        case 308: {
            // Stop and reset timeline
            gml = "timeline_running=0;timeline_position=0;";
            break;
        }
        case 309: {
            // Set timeline speed
            gml = "timeline_speed=argument[0]";
            break;
        }
        case 321: {
            // Show message
            gml = "show_message(argument[0])";
            break;
        }
        case 404: {
            // Test the number of instances
            gml = "instance_number(argument[0])";
            const char* op;
            switch (args[2][0]) {
                case '0':
                    op = _not ? "!=" : "==";
                    break;
                case '1':
                    op = _not ? ">=" : "<";
                    break;
                case '2':
                    op = _not ? "<=" : ">";
                    break;
                default:
                    return false;
            }
            gml += op;
            gml += "argument[1]";
            break;
        }
        case 405: {
            // 1 in n chance to perform the next action
            if(_not) 
                gml = "random(argument[0]) > 1";
            else
                gml = "random(argument[0]) < 1";
            break;
        }
        case 408: {
            // If expression is true
            if (_not)
                gml = "argument[0]";
            else
                gml = "!argument[0]";
            break;
        }
        case 421: {
            // Else
            break;
        }
        case 422: {
            // Start of a block
            break;
        }
        case 423: {
            // Repeat n times
            action.param = std::atoi(args[0]);
            break;
        }
        case 424: {
            // End of a block
            break;
        }
        case 501: {
            if(relative) gml = "draw_sprite(argument[0],argument[3],x+argument[1],y+argument[2])";
            else gml = "draw_sprite(argument[0],argument[3],argument[1],argument[2])";
            break;
        }
        case 514: {
            const char* arg0 = args[0];
            const char* arg1 = args[1];
            const char* arg2 = args[2];
            if(relative) gml = "draw_text(x+argument[1],y+argument[2],argument[0])";
            else gml = "draw_text(argument[1],argument[2],argument[0])";
            break;
        }
        case 524: {
            // Set the drawing colour
            gml = "draw_set_color(argument[0])";
            break;
        }
        case 526: {
            // Set the font
            gml = "draw_set_color(argument[0]);draw_set_halign(argument[1]);";
            break;
        }
        case 531: {
            // Set fullscreen mode
            gml = "if(argument[0])window_set_fullscreen(argument[0]==2)else window_set_fullscreen(!window_get_fullscreen())";
            break;
        }
        case 532: {
            // Create an effect
            if (args[5][0] == '0')
                gml = "effect_create_below";
            else
                gml = "effect_create_above";
            if (relative)
                gml += "(argument[0],x+argument[1],y+argument[2],argument[3],argument[4])";
            else
                gml += "(argument[0],argument[1],argument[2],argument[3],argument[4])";
            break;
        }
        case 541: {
            // Change sprite
            gml = "sprite_index=argument[0];image_index=argument[1];image_speed=argument[2];";
            break;
        }
        case 542: {
            // Transform the sprite
            bool xflip = args[3][0] == '1' || args[3][0] == '3';
            bool yflip = args[3][0] == '2' || args[3][0] == '3';
            gml = "image_xscale=";
            if (xflip) gml += "-";
            gml += "argument[0];image_yscale=";
            if (yflip) gml += "-";
            gml += "argument[1];image_angle=argument[2];";
            break;
        }
        case 601: {
            // Execute a script
            gml = "script_execute(argument[0],argument[1],argument[2],argument[3],argument[4],argument[5])";
            break;
        }
        case 603: {
            // Execute a block of code
            break;
        }
        case 604: {
            // Call the inherited event
            gml = "event_inherited()";
            break;
        }
        case 605: {
            // Comment
            break;
        }
        case 611: {
            // Set variable
            gml = args[0];
            gml += relative ? "+=" : "=";
            gml += args[1];

            free(args[0]);
            free(args[1]);
            action.paramCount = 0;
            delete action.params[0];
            delete action.params[1];
            break;
        }
        case 612: {
            // Test variable
            gml = args[0];
            const char* op;
            switch (args[2][0]) {
                case '0':
                    op = _not ? "!=" : "==";
                    break;
                case '1':
                    op = _not ? ">=" : "<";
                    break;
                case '2':
                    op = _not ? "<=" : ">";
                    break;
            }
            gml += op;
            gml += "argument[1]";
            break;
        }
        case 721: {
            // Set health
            if (relative)
                gml = "health+=argument[0]";
            else
                gml = "health=argument[0]";
            break;
        }
        default: {
            // Unknown
            return false;
        }
    }

    // Register the code we just generated
    if (action.question) {
        action.codeObj = CodeManager::RegisterQuestion(gml.c_str(), static_cast<unsigned int>(strlen(gml.c_str())));
    }
    else {
        action.codeObj = CodeManager::Register(gml.c_str(), static_cast<unsigned int>(strlen(gml.c_str())));
    }

    // Clean up
    (*out) = ( CodeAction )_actions.size();
    _actions.push_back(action);
    for (i = 0; i < action.paramCount; i++) {
        free(args[i]);
    }
    delete[] args;
    return true;
}

bool CodeActionManager::Compile(CodeAction action) {
    for (unsigned int i = 0; i < _actions[action].paramCount; i++) {
        if(!_actions[action].params[i]->Compile()) {
            return false;
        }
    }
    return CodeManager::Compile(_actions[action].codeObj);
}

bool CodeActionManager::Run(CodeAction* actions, unsigned int count, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId) {
    unsigned int pos = 0;
    while (pos < count) {
        bool run = true;
        GMLType args[8];

        if (_actions[actions[pos]].question) {
            // Multiple questions can be chained together with the end statement dependent on all of them.
            while (_actions[actions[pos]].question) {
                if (run) {
                    for (unsigned int i = 0; i < _actions[actions[pos]].paramCount; i++) {
                        _actions[actions[pos]].params[i]->Evaluate(self, other, ev, sub, asObjId, &args[i]);
                    }
                    bool r;
                    if (!CodeManager::Query(_actions[actions[pos]].codeObj, self, other, ev, sub, asObjId, &r, _actions[actions[pos]].paramCount, args)) return false;
                    run &= r;
                }
                pos++;
            }
        }

        if (run) {
            if (_actions[actions[pos]].appliesToSomething && _actions[actions[pos]].appliesTo != -1) {
                if (_actions[actions[pos]].appliesTo == -2) {
                    for (unsigned int i = 0; i < _actions[actions[pos]].paramCount; i++) {
                        if(!_actions[actions[pos]].params[i]->Evaluate(other, self, ev, sub, other->object_index, &args[i])) return false;
                    }
                    if (!CodeManager::Run(_actions[actions[pos]].codeObj, other, self, ev, sub, other->object_index, _actions[actions[pos]].paramCount, args)) return false;
                }
                else {
                    InstanceList::Iterator iter(_actions[actions[pos]].appliesTo);
                    Instance* inst;
                    while (inst = iter.Next()) {
                        for (unsigned int i = 0; i < _actions[actions[pos]].paramCount; i++) {
                            if(!_actions[actions[pos]].params[i]->Evaluate(inst, self, ev, sub, inst->object_index, &args[i])) return false;
                        }
                        if (!CodeManager::Run(_actions[actions[pos]].codeObj, inst, self, ev, sub, inst->object_index, _actions[actions[pos]].paramCount, args)) return false;
                    }
                }
            }
            else {
                for (unsigned int i = 0; i < _actions[actions[pos]].paramCount; i++) {
                    if(!_actions[actions[pos]].params[i]->Evaluate(self, other, ev, sub, asObjId, &args[i])) return false;
                }
                if (!CodeManager::Run(_actions[actions[pos]].codeObj, self, other, ev, sub, asObjId, _actions[actions[pos]].paramCount, args)) return false;
            }
            pos++;
        }
        else {
            // Not a question and we are NOT going to run it.
            if (_actions[actions[pos]].actionID == 422) {
                // "start block", so skip until matching end block
                pos++;
                unsigned int depth = 1;
                while (depth) {
                    if (_actions[actions[pos]].actionID == 422)
                        depth++;
                    else if (_actions[actions[pos]].actionID == 424)
                        depth--;
                    pos++;
                }
            }
            else {
                pos++;
            }
        }
    }
    return true;
}


bool CodeActionManager::RunInstanceEvent(int ev, int sub, Instance* target, Instance* other, unsigned int asObjId) {
    Object* o = AssetManager::GetObject(asObjId);
    while (!CheckObjectEvent(ev, sub, o)) {
        if (o->parentIndex < 0) return true;
        o = AssetManager::GetObject(o->parentIndex);
    }
    return o->events[ev].count(sub) ? Run(o->events[ev][sub].actions, o->events[ev][sub].actionCount, target, other, ev, sub, asObjId) : true;
}


bool CodeActionManager::CheckObjectEvent(int ev, int sub, Object* o) { return o->events[ev].count(sub); }
