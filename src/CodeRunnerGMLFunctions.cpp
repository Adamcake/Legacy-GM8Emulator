#include <pch.h>

#include "AssetManager.hpp"
#include "CRGMLType.hpp"
#include "CodeActionManager.hpp"
#include "CodeRunner.hpp"
#include "Collision.hpp"
#include "Compiler/CRRuntime.hpp"
#include "File.hpp"
#include "GlobalValues.hpp"
#include "InputHandler.hpp"
#include "Instance.hpp"
#include "InstanceList.hpp"
#include "RNG.hpp"
#include "Renderer.hpp"

// Private vars
namespace Runtime {
    // "draw" vars
    unsigned int _drawFont = -1;
    int _drawColour = 0;
    int _drawValign = 0;
    int _drawHalign = 0;
    double _drawAlpha = 1.0;

    // User files
    std::fstream _userFiles[32];

    // Room order
    unsigned int** _roomOrder;
    unsigned int _roomOrderCount;
    void SetRoomOrder(unsigned int** order, unsigned int count) {
        _roomOrder = order;
        _roomOrderCount = count;
    }
}

/*
All GML functions have this format:
bool <gml_function_name>(unsigned int argc, GMLType* argv, GMLType* out)

argc is the number of arguments being passed. argv is the array of arguments.
out is where you should return the GML value being returned by the function.
The function should return true on success, false on failure (ie. game should close.)

Note that argc hasn't been verified as correct when passed in. So, for example, instance_create isn't guaranteed to have
exactly 3 arguments, even though it's always meant to. So it should return false if it doesn't have the correct argc.
That said, it SHOULD always be correct unless some gamedata has been manufactured otherwise, so we'll omit checks for efficiency.

Your implementation must account for "out" being NULL. For example, instance_create() should output the new instance id,
but if out is NULL then it doesn't output anything, as the runner has indicated that the output isn't required.
*/

bool Runtime::abs(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::abs(argv[0].dVal);
    }
    return true;
}

bool Runtime::arcsin(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::asin(argv[0].dVal);
    }
    return true;
}

bool Runtime::arccos(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::acos(argv[0].dVal);
    }
    return true;
}

bool Runtime::arctan(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::atan(argv[0].dVal);
    }
    return true;
}

bool Runtime::ceil(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::ceil(argv[0].dVal);
    }
    return true;
}

bool Runtime::choose(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!argc) {
        if (out) {
            out->state = GMLTypeState::Double;
            out->dVal = GMLFalse;
        }
        return true;
    }

    int rand = RNG::Irandom(argc - 1);
    if(out) (*out) = argv[rand];
    return true;
}

bool Runtime::cos(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::cos(argv[0].dVal);
    }
    return true;
}

bool Runtime::degtorad(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = (GML_PI * argv[0].dVal) / 180.0;
    }
    return true;
}

bool Runtime::distance_to_object(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    Instance* self = GetContext().self;
    Instance* other;
    InstanceList::Iterator iter(_round(argv[0].dVal));
    double lowestDist = 1000000.0;  // GML default
    RefreshInstanceBbox(self);

    while (other = iter.Next()) {
        RefreshInstanceBbox(other);

        int distanceAbove = other->bbox_top - self->bbox_bottom;
        int distanceBelow = self->bbox_top - other->bbox_bottom;
        unsigned int absHeightDiff = 0;
        if (distanceAbove > 0)
            absHeightDiff = distanceAbove;
        else if (distanceBelow > 0)
            absHeightDiff = distanceBelow;

        int distanceLeft = other->bbox_left - self->bbox_right;
        int distanceRight = self->bbox_left - other->bbox_right;
        unsigned int absSideDiff = 0;
        if (distanceLeft > 0)
            absSideDiff = distanceLeft;
        else if (distanceRight > 0)
            absSideDiff = distanceRight;

        double absDist = 0.0;
        if (absSideDiff || absHeightDiff) {
            absDist = ::sqrt((absSideDiff * absSideDiff) + (absHeightDiff * absHeightDiff));
            if (absDist < lowestDist) lowestDist = absDist;
        }
        else {
            lowestDist = 0.0;
            break;
        }
    }

    out->state = GMLTypeState::Double;
    out->dVal = lowestDist;

    return true;
}

bool Runtime::draw_rectangle(unsigned int argc, GMLType* argv, GMLType* out) {
    // todo
    return true;
}

bool Runtime::draw_set_alpha(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    _drawAlpha = argv[0].dVal;
    return true;
}

bool Runtime::draw_set_color(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    _drawColour = _round(argv[0].dVal);
    return true;
}

bool Runtime::draw_set_font(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    _drawFont = _round(argv[0].dVal);
    return true;
}

bool Runtime::draw_set_halign(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    _drawHalign = _round(argv[0].dVal);
    return true;
}

bool Runtime::draw_set_valign(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    _drawValign = _round(argv[0].dVal);
    return true;
}

bool Runtime::draw_sprite(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 4, true, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    Sprite* spr = AssetManager::GetSprite(_round(argv[0].dVal));
    Instance* self = GetContext().self;
    int frame = _round(argv[1].dVal);
    if (frame < 0) frame = self->image_index;
    RDrawImage(spr->frames[frame % spr->frameCount], argv[2].dVal, argv[3].dVal, self->image_xscale, self->image_yscale, self->image_angle, self->image_blend, self->image_alpha, self->depth);
    return true;
}

bool Runtime::draw_sprite_ext(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 9, true, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double,
            GMLTypeState::Double, GMLTypeState::Double))
        return false;
    Sprite* spr = AssetManager::GetSprite(_round(argv[0].dVal));
    Instance* self = GetContext().self;
    int frame = _round(argv[1].dVal);
    if(frame < 0) frame = self->image_index;
    RDrawImage(spr->frames[frame % spr->frameCount], argv[2].dVal, argv[3].dVal, argv[4].dVal, argv[5].dVal, argv[6].dVal, _round(argv[7].dVal), argv[8].dVal, self->depth);
    return true;
}

bool Runtime::draw_text(unsigned int argc, GMLType* argv, GMLType* out) {
    if (argc != 3) return false;
    const char* str = argv[2].sVal.c_str();
    std::string st;
    if (argv[2].state == GMLTypeState::Double) {
        std::stringstream ss;
        ss.precision(_round(argv[2].dVal) == argv[2].dVal ? 0 : 2);
        ss << std::fixed << argv[2].dVal;
        st = ss.str();
        str = st.c_str();
    }

    Font* font = AssetManager::GetFont(_drawFont);
    if (font && font->exists) {
        int cursorX = _round(argv[0].dVal);
        int cursorY = _round(argv[1].dVal);

        unsigned tallest = 0;
        for (const char* pC = str; (*pC) != '\0'; pC++) {
            const char c = *pC;
            if (c == '#' && (pC == str || *(pC - 1) != '\\')) continue;
            unsigned int h = font->dmap[(c * 6) + 3];
            if (h > tallest) tallest = h;
        }

        if (_drawValign == 1 || _drawValign == 2) {
            unsigned int lineCount = 1;
            for (const char* pC = str; (*pC) != '\0'; pC++) {
                const char c = *pC;
                if (c == '#' && (pC == str || *(pC - 1) != '\\')) {
                    lineCount++;
                }
            }
            lineCount = lineCount * tallest;
            if (_drawValign == 1) lineCount /= 2;
            cursorY -= lineCount;
        }

        bool recalcX = true;
        for (const char* pC = str; (*pC) != '\0'; pC++) {
            const char c = *pC;
            if (c == '#' && (pC == str || *(pC - 1) != '\\')) {
                recalcX = true;
                cursorY += tallest;
                continue;
            }
            else if (c == '\\' && *(pC + 1) == '#') {
                continue;
            }

            if (recalcX) {
                cursorX = _round(argv[0].dVal);
                if (_drawHalign == 1 || _drawHalign == 2) {
                    unsigned int lineWidth = 0;
                    for (const char* tC = pC; (*tC) != '\0'; tC++) {
                        if ((*tC) == '#' && (tC == str || *(tC - 1) != '\\')) break;
                        if ((*tC) == '\\' && *(tC + 1) == '#') continue;
                        lineWidth += font->dmap[((*tC) * 6) + 4];
                    }
                    if (_drawHalign == 1) lineWidth /= 2;
                    cursorX -= lineWidth;
                }
                recalcX = false;
            }

            if (font->rangeBegin <= ( unsigned int )c && font->rangeEnd >= ( unsigned int )c) {
                unsigned int* dmapPos = font->dmap + (c * 6);
                unsigned int cX = *(dmapPos);
                unsigned int cY = *(dmapPos + 1);
                unsigned int cW = *(dmapPos + 2);
                unsigned int cH = *(dmapPos + 3);
                unsigned int cCW = *(dmapPos + 4);
                unsigned int cCO = *(dmapPos + 5);

                RDrawPartialImage(font->image, cursorX + ( int )cCO, cursorY, 1, 1, 0.0, _drawColour, _drawAlpha, cX, cY, cW, cH, GetContext().self->depth);
                cursorX += cCW;
            }
        }
    }
    else {
        // Should use the default font here
    }

    return true;
}

bool Runtime::event_inherited(unsigned int argc, GMLType* argv, GMLType* out) {
    Object* o = AssetManager::GetObject(GetContext().objId);
    unsigned int id;

    while (o->parentIndex >= 0) {
        id = o->parentIndex;
        o = AssetManager::GetObject(id);

        if (CodeActionManager::CheckObjectEvent(GetContext().eventId, GetContext().eventNumber, o)) {
            return CodeActionManager::RunInstanceEvent(GetContext().eventId, GetContext().eventNumber, GetContext().self, GetContext().other, id);
        }
    }
    return true;
}

bool Runtime::event_perform(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    return CodeActionManager::RunInstanceEvent(_round(argv[0].dVal), _round(argv[1].dVal), GetContext().self, GetContext().other, GetContext().self->object_index);
}


// --- FILE ---
// removed filesystem for now because mingw is a broken piece of shit
bool fsExists(std::string& path) {
    std::ifstream ifs(path);
    return ifs.good() && ifs.is_open();
}

bool Runtime::file_bin_open(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::String, GMLTypeState::Double)) return false;
    // fs::path filePath = fs::path(argv[0].sVal);

    ::std::string filePath = argv[0].sVal;
    int fileType = _round(argv[1].dVal);

    if (fsExists(filePath) || (fileType != 0)) {
        // Stream mode
        int fileMode;
        switch (fileType) {
            case 0:  // Read-only
                fileMode = std::fstream::in | std::fstream::binary;
                break;
            case 1:  // Write-only
                fileMode = std::fstream::out | std::fstream::binary;
                break;
            case 2:
            default:  // Read & write
                fileMode = std::fstream::in | std::fstream::out | std::fstream::binary;
                break;
        }

        int i = 0;
        for (; i < maxFilesOpen; i++) {
            if (!_userFiles[i].is_open()) {
                _userFiles[i].open(filePath, static_cast<std::ios_base::openmode>(fileMode));
                break;
            }
        }

        if (i == maxFilesOpen) {
            return false;
        }
        else if (out) {
            out->state = GMLTypeState::Double;
            out->dVal = static_cast<double>(i + 1);
        }

        return true;
    }
    else {
        return false;  // I guess?
    }
}

bool Runtime::file_bin_close(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    int idx = _round(argv[0].dVal);
    if (_userFiles[idx - 1].is_open()) {
        _userFiles[idx - 1].close();
        return true;
    }
    else {
        return false;
    }
}

bool Runtime::file_bin_read_byte(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    int idx = _round(argv[0].dVal);

    if (_userFiles[idx - 1].is_open()) {
        if (!_userFiles[idx - 1].eof()) {
            if (out) {
                out->state = GMLTypeState::Double;
                out->dVal = static_cast<double>(_userFiles[idx - 1].get());
            }
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

bool Runtime::file_bin_write_byte(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    int idx = _round(argv[0].dVal);
    int oByte = _round(argv[1].dVal);

    if (_userFiles[idx - 1].is_open()) {
        _userFiles[idx - 1].put(static_cast<char>(oByte));
        return true;
    }
    else {
        return false;
    }
}

bool Runtime::file_delete(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, false, GMLTypeState::String)) return false;
    return std::remove(argv[0].sVal.c_str()) == 0;
}

bool Runtime::file_exists(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::String)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        // (fs::exists(fs::path(argv[0].sVal))
        out->dVal = (fsExists(argv[0].sVal) ? GMLTrue : GMLFalse);
    }
    return true;
}


// --- FILE END ---


bool Runtime::instance_change(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, false, GMLTypeState::Double, GMLTypeState::Double)) return false;
    bool events = _isTrue(&argv[1]);
    Instance* i = GetContext().self;
    if(events) {
        if (!CodeActionManager::RunInstanceEvent(1, 0, i, NULL, i->object_index)) return false;
    }
    int objId = _round(argv[0].dVal);
    if(objId < 0 || objId >= static_cast<int>(AssetManager::GetObjectCount())) {
        Runtime::SetReturnCause(ReturnCause::ExitError);
        Runtime::PushErrorMessage("Invalid object index passed to instance_change");
        return false;
    }
    Object* obj = AssetManager::GetObject(objId);
    if(!obj->exists) {
        Runtime::SetReturnCause(ReturnCause::ExitError);
        Runtime::PushErrorMessage("Non-existent object passed to instance_change");
        return false;
    }
    i->object_index = objId;
    i->solid = obj->solid;
    i->visible = obj->visible;
    i->persistent = obj->persistent;
    i->depth = obj->depth;
    i->sprite_index = obj->spriteIndex;
    i->mask_index = obj->maskIndex;
    if(events) {
        if (!CodeActionManager::RunInstanceEvent(0, 0, i, NULL, i->object_index)) return false;
    }
    return true;
}

bool Runtime::instance_create(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 3, false, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    unsigned int objID = _round(argv[2].dVal);
    Instance* i = InstanceList::AddInstance(argv[0].dVal, argv[1].dVal, objID);
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = i->id;
    }
    if (!CodeActionManager::RunInstanceEvent(0, 0, i, NULL, i->object_index)) return false;
    return true;
}

bool Runtime::instance_destroy(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!CodeActionManager::RunInstanceEvent(1, 0, GetContext().self, NULL, GetContext().self->object_index)) return false;
    GetContext().self->exists = false;
    return true;
}

bool Runtime::instance_exists(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    int objId = _round(argv[0].dVal);
    InstanceList::Iterator it(( unsigned int )objId);
    out->state = GMLTypeState::Double;
    out->dVal = (it.Next() ? GMLTrue : GMLFalse);
    return true;
}

bool Runtime::instance_number(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    int objId = _round(argv[0].dVal);
    InstanceList::Iterator it(( unsigned int )objId);
    out->state = GMLTypeState::Double;
    unsigned int count = 0;
    while (it.Next()) count++;
    out->dVal = ( double )count;
    return true;
}

bool Runtime::instance_position(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 3, true, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        int objId = _round(argv[2].dVal);
        int x = _round(argv[0].dVal);
        int y = _round(argv[1].dVal);
        InstanceList::Iterator it(( unsigned int )objId);
        if (objId == -3) it = InstanceList::Iterator();
        Instance* instance;
        double ret = -4.0;
        while (instance = it.Next()) {
            if (CollisionPointCheck(instance, x, y)) {
                ret = instance->id;
                break;
            }
        }
        out->state = GMLTypeState::Double;
        out->dVal = ret;
    }
    return true;
}

bool Runtime::irandom(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, false, GMLTypeState::Double)) return false;
    int rand = RNG::Irandom(_round(argv[0].dVal));
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ( double )rand;
    }
    return true;
}

bool Runtime::irandom_range(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    int rand = RNG::Irandom(::abs(_round(argv[1].dVal) - _round(argv[0].dVal)) + 1);
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ( double )rand + std::fmin(argv[0].dVal, argv[1].dVal);
    }
    return true;
}

bool Runtime::is_real(unsigned int argc, GMLType* argv, GMLType* out) {
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = (argv[0].state == GMLTypeState::Double) ? GMLTrue : GMLFalse;
    }
    return true;
}

bool Runtime::is_string(unsigned int argc, GMLType* argv, GMLType* out) {
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = (argv[0].state == GMLTypeState::String) ? GMLTrue : GMLFalse;
    }
    return true;
}

bool Runtime::floor(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::floor(argv[0].dVal);
    }
    return true;
}

bool Runtime::game_end(unsigned int argc, GMLType* argv, GMLType* out) {
    SetReturnCause(ReturnCause::ExitGameEnd);
    return false;
}

bool Runtime::game_restart(unsigned int argc, GMLType* argv, GMLType* out) {
    GetGlobals()->changeRoom = true;
    GetGlobals()->roomTarget = (*_roomOrder)[0];
    InstanceList::Iterator iter;
    Instance* i;
    while (i = iter.Next()) {
        i->exists = false;
    }
    return true;
}

bool Runtime::keyboard_check(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    out->state = GMLTypeState::Double;
    int gmlKeycode = _round(argv[0].dVal);
    out->dVal = (InputCheckKey(gmlKeycode) ? GMLTrue : GMLFalse);
    return true;
}

bool Runtime::keyboard_check_direct(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    out->state = GMLTypeState::Double;
    int gmlKeycode = _round(argv[0].dVal);
    out->dVal = (InputCheckKeyDirect(gmlKeycode) ? GMLTrue : GMLFalse);
    return true;
}

bool Runtime::keyboard_check_pressed(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    out->state = GMLTypeState::Double;
    int gmlKeycode = _round(argv[0].dVal);
    out->dVal = (InputCheckKeyPressed(gmlKeycode) ? GMLTrue : GMLFalse);
    return true;
}

bool Runtime::keyboard_check_released(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    out->state = GMLTypeState::Double;
    int gmlKeycode = _round(argv[0].dVal);
    out->dVal = (InputCheckKeyReleased(gmlKeycode) ? GMLTrue : GMLFalse);
    return true;
}

bool Runtime::lengthdir_x(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if(out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::cos(argv[1].dVal * GML_PI / 180.0) * argv[0].dVal;
    }
    return true;
}

bool Runtime::lengthdir_y(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = (-::sin(argv[1].dVal * GML_PI / 180.0)) * argv[0].dVal;
    }
    return true;
}

bool Runtime::log2(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::log2(argv[0].dVal);
    }
    return true;
}

bool Runtime::log10(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::log10(argv[0].dVal);
    }
    return true;
}

bool Runtime::logn(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::log(argv[1].dVal) / ::log(argv[0].dVal);
    }
    return true;
}

bool Runtime::ln(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::log(argv[0].dVal);
    }
    return true;
}

bool Runtime::make_color_hsv(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 3, false, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        float fH = ( float )((argv[0].dVal / 255.0) * 360.0);
        float fS = ( float )(argv[1].dVal / 255.0);
        float fV = ( float )(argv[2].dVal / 255.0);
        float fR, fG, fB;
        float fC = fV * fS;  // Chroma
        float fHPrime = ( float )fmod(fH / 60.0, 6);
        float fX = fC * ( float )(1 - fabs(fmod(fHPrime, 2) - 1));
        float fM = fV - fC;

        if (0 <= fHPrime && fHPrime < 1) {
            fR = fC;
            fG = fX;
            fB = 0;
        }
        else if (1 <= fHPrime && fHPrime < 2) {
            fR = fX;
            fG = fC;
            fB = 0;
        }
        else if (2 <= fHPrime && fHPrime < 3) {
            fR = 0;
            fG = fC;
            fB = fX;
        }
        else if (3 <= fHPrime && fHPrime < 4) {
            fR = 0;
            fG = fX;
            fB = fC;
        }
        else if (4 <= fHPrime && fHPrime < 5) {
            fR = fX;
            fG = 0;
            fB = fC;
        }
        else if (5 <= fHPrime && fHPrime < 6) {
            fR = fC;
            fG = 0;
            fB = fX;
        }
        else {
            fR = 0;
            fG = 0;
            fB = 0;
        }

        fR += fM;
        fG += fM;
        fB += fM;

        out->state = GMLTypeState::Double;
        out->dVal = (( unsigned int )(fR * 255)) | ((( unsigned int )(fG * 255)) << 8) | ((( unsigned int )(fB * 255)) << 16);
    }
    return true;
}

bool Runtime::make_color_rgb(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 3, false, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if(out) {
        out->state = GMLTypeState::Double;
        out->dVal = _round(argv[0].dVal) + (_round(argv[1].dVal) << 8) + (_round(argv[2].dVal) << 16);
    }
    return true;
}

bool Runtime::max(unsigned int argc, GMLType* argv, GMLType* out) {
    if (out) {
        GMLType ret;
        if (argc == 0) {
            (*out) = ret;
            return true;
        }

        ret = *argv;
        for (GMLType* arg = argv + 1; arg < (argv + argc); arg++) {
            if (arg->state == GMLTypeState::String && ret.state == GMLTypeState::String) {
                unsigned int c = 0;
                while (true) {
                    if ((*(arg->sVal.c_str() + c)) > (*(ret.sVal.c_str() + c))) {
                        ret = *arg;
                        break;
                    }
                    if ((*(arg->sVal.c_str() + c)) == '\0') {
                        break;
                    }
                    c++;
                }
            }
            else if (arg->state == GMLTypeState::Double && ret.state == GMLTypeState::Double) {
                if (arg->dVal > ret.dVal) ret = *arg;
            }
        }
        (*out) = ret;
    }
    return true;
}

bool Runtime::min(unsigned int argc, GMLType* argv, GMLType* out) {
    if (out) {
        GMLType ret;
        if (argc == 0) {
            (*out) = ret;
            return true;
        }

        ret = *argv;
        for (GMLType* arg = argv + 1; arg < (argv + argc); arg++) {
            if (arg->state == GMLTypeState::String && ret.state == GMLTypeState::String) {
                unsigned int c = 0;
                while (true) {
                    if ((*(arg->sVal.c_str() + c)) < (*(ret.sVal.c_str() + c))) {
                        ret = *arg;
                        break;
                    }
                    if ((*(ret.sVal.c_str() + c)) == '\0') {
                        break;
                    }
                    c++;
                }
            }
            else if (arg->state == GMLTypeState::Double && ret.state == GMLTypeState::Double) {
                if (arg->dVal < ret.dVal) ret = *arg;
            }
            else {
                ret = *arg;
            }
        }
        (*out) = ret;
    }
    return true;
}

bool Runtime::motion_set(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    Instance* self = GetContext().self;
    self->direction = argv[0].dVal;
    self->speed = argv[1].dVal;
    return true;
}

bool Runtime::move_bounce_solid(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (_isTrue(argv)) {
        // advanced bouncing - TODO
        return false;
    }
    else {
        // Basic bouncing
        Instance* self = GetContext().self;
        double startx = self->x, starty = self->y;
        Instance* target;

        // First collision check - x offset only
        self->x += self->hspeed;
        self->bboxIsStale = true;
        bool didChange = false;
        InstanceList::Iterator iter;
        while (target = iter.Next()) {
            if (target->solid) {
                if (CollisionCheck(self, target)) {
                    self->hspeed = -self->hspeed;
                    didChange = true;
                    break;
                }
            }
        }

        // Second collision check - y offset only
        self->x = startx;
        self->y += self->vspeed;
        self->bboxIsStale = true;
        iter = InstanceList::Iterator();
        while (target = iter.Next()) {
            if (target->solid) {
                if (CollisionCheck(self, target)) {
                    self->vspeed = -self->vspeed;
                    didChange = true;
                    break;
                }
            }
        }

        if (!didChange) {
            // Third collision check - x and y offset
            self->x += self->hspeed;
            self->bboxIsStale = true;
            iter = InstanceList::Iterator();
            while (target = iter.Next()) {
                if (target->solid) {
                    if (CollisionCheck(self, target)) {
                        self->hspeed = -self->hspeed;
                        self->vspeed = -self->vspeed;
                        didChange = true;
                        break;
                    }
                }
            }
        }

        self->x = startx;
        self->y = starty;
        self->bboxIsStale = true;

        if (didChange) {
            self->direction = ::atan2(-self->vspeed * GML_PI / 180.0, self->hspeed * GML_PI / 180.0) * 180.0 / GML_PI;
            self->speed = ::sqrt(pow(self->hspeed, 2) + pow(self->vspeed, 2));
        }

        return true;
    }
}

bool Runtime::move_contact_solid(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    int maxdist = _round(argv[1].dVal);
    if (maxdist <= 0) maxdist = 1000;  // GML default
    double hspeed = ::cos(argv[0].dVal * GML_PI / 180.0);
    double vspeed = -::sin(argv[0].dVal * GML_PI / 180.0);
    Instance* self = GetContext().self;
    bool moved = false;

    for (int i = 0; i <= maxdist; i++) {
        InstanceList::Iterator iter;
        bool collision = false;

        Instance* target;
        while (target = iter.Next()) {
            if ((target != self) && target->solid) {
                if (CollisionCheck(self, target)) {
                    collision = true;
                    break;
                }
            }
        }

        if (collision) {
            if (moved) {
                self->x -= hspeed;
                self->y -= vspeed;
                self->bboxIsStale = true;
            }
            break;
        }
        else {
            if (i != maxdist) {
                self->x += hspeed;
                self->y += vspeed;
                self->bboxIsStale = true;
                moved = true;
            }
            else {
                break;
            }
        }
    }
    return true;
}

bool Runtime::move_towards_point(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 3, true, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    Instance* i = GetContext().self;
    i->direction = (::atan2((i->y - argv[1].dVal), (argv[0].dVal - i->x))) * 180.0 / GML_PI;
    i->speed = argv[2].dVal;
    i->hspeed = ::cos(i->direction * GML_PI / 180.0) * i->speed;
    i->vspeed = -::sin(i->direction * GML_PI / 180.0) * i->speed;
    return true;
}

bool Runtime::move_wrap(unsigned int argc, GMLType* argv, GMLType* out) {
    if (argv[2].state == GMLTypeState::String) return false;
    bool hor = _isTrue(argv + 0);
    bool ver = _isTrue(argv + 1);
    double margin = argv[2].dVal;
    Instance* instance = GetContext().self;

    if (hor) {
        unsigned int roomW = AssetManager::GetRoom(GetGlobals()->room)->width;
        if (instance->x < -margin) {
            instance->x += roomW;
        }
        else if (instance->x >= (roomW + margin)) {
            instance->x -= roomW;
        }
    }

    if (ver) {
        unsigned int roomH = AssetManager::GetRoom(GetGlobals()->room)->height;
        if (instance->y < -margin) {
            instance->y += roomH;
        }
        else if (instance->y >= (roomH + margin)) {
            instance->y -= roomH;
        }
    }

    return true;
}

bool Runtime::ord(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ( double )argv[0].sVal[0];
    }
    return true;
}

bool Runtime::place_free(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = GMLTrue;

        InstanceList::Iterator iter;
        Instance* self = GetContext().self;
        double oldX = self->x;
        double oldY = self->y;
        self->x = argv[0].dVal;
        self->y = argv[1].dVal;
        self->bboxIsStale = true;

        Instance* target;
        while (target = iter.Next()) {
            if ((target != self) && target->solid) {
                if (CollisionCheck(self, target)) {
                    out->dVal = GMLFalse;
                    break;
                }
            }
        }

        self->x = oldX;
        self->y = oldY;
        self->bboxIsStale = true;
    }
    return true;
}

bool Runtime::place_meeting(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 3, true, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = GMLFalse;
        int obj = _round(argv[2].dVal);
        InstanceList::Iterator iter(( unsigned int )obj);
        if (obj == -3) iter = InstanceList::Iterator();

        Instance* self = GetContext().self;
        double oldX = self->x;
        double oldY = self->y;
        self->x = argv[0].dVal;
        self->y = argv[1].dVal;
        self->bboxIsStale = true;

        Instance* target;
        while (target = iter.Next()) {
            if (target != self) {
                if (CollisionCheck(self, target)) {
                    out->dVal = GMLTrue;
                    break;
                }
            }
        }

        self->x = oldX;
        self->y = oldY;
        self->bboxIsStale = true;
    }
    return true;
}

bool Runtime::point_distance(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 4, true, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::sqrt(::pow(argv[1].dVal - argv[3].dVal, 2) + ::pow(argv[2].dVal - argv[0].dVal, 2));
    }
    return true;
}

bool Runtime::point_direction(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 4, true, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = (::atan2((argv[1].dVal - argv[3].dVal), (argv[2].dVal - argv[0].dVal))) * 180.0 / GML_PI;
    }
    return true;
}

bool Runtime::power(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, true, GMLTypeState::Double, GMLTypeState::Double)) return false;
    if (out) {
        (*out).state = GMLTypeState::Double;
        (*out).dVal = ::pow(argv[0].dVal, argv[1].dVal);
    }
    return true;
}

bool Runtime::radtodeg(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = (180.0 * argv[0].dVal) / GML_PI;
    }
    return true;
}

bool Runtime::random(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, false, GMLTypeState::Double)) return false;
    double rand = RNG::Random(argv[0].dVal);
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = rand;
    }
    return true;
}

bool Runtime::random_range(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 2, false, GMLTypeState::Double, GMLTypeState::Double)) return false;
    double rand = RNG::Random(argv[1].dVal - argv[0].dVal);
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = rand + argv[0].dVal;
    }
    return true;
}

bool Runtime::random_get_seed(unsigned int argc, GMLType* argv, GMLType* out) {
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ( double )RNG::GetSeed();
    }
    return true;
}

bool Runtime::random_set_seed(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    RNG::SetSeed(_round(argv[0].dVal));
    return true;
}

bool Runtime::room_goto(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, false, GMLTypeState::Double)) return false;
    GetGlobals()->changeRoom = true;
    GetGlobals()->roomTarget = _round(argv[0].dVal);

    return true;
}

bool Runtime::room_goto_next(unsigned int argc, GMLType* argv, GMLType* out) {
    GetGlobals()->changeRoom = true;
    if ((*_roomOrder)[_roomOrderCount - 1] == GetGlobals()->room) return false;  // Trying to go to next room from last room
    for (unsigned int i = 0; i < _roomOrderCount; i++) {
        if ((*_roomOrder)[i] == GetGlobals()->room) {
            GetGlobals()->roomTarget = (*_roomOrder)[i + 1];
            return true;
        }
    }
    return false;
}

bool Runtime::room_goto_previous(unsigned int argc, GMLType* argv, GMLType* out) {
    GetGlobals()->changeRoom = true;
    if ((*_roomOrder)[0] == GetGlobals()->room) return false;  // Trying to go to next room from last room
    for (unsigned int i = 0; i < _roomOrderCount; i++) {
        if ((*_roomOrder)[i] == GetGlobals()->room) {
            GetGlobals()->roomTarget = (*_roomOrder)[i - 1];
            return true;
        }
    }
    return false;
}

bool Runtime::round(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = _round(argv[0].dVal);
    }
    return true;
}

bool Runtime::sign(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = (argv[0].dVal == 0 ? 0 : (argv[0].dVal < 0 ? -1 : 1));
    }
    return true;
}

bool Runtime::sin(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::sin(argv[0].dVal);
    }
    return true;
}

bool Runtime::sqr(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = argv[0].dVal * argv[0].dVal;
    }
    return true;
}

bool Runtime::sqrt(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (argv[0].dVal < 0) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::sqrt(argv[0].dVal);
    }
    return true;
}

bool Runtime::string(unsigned int argc, GMLType* argv, GMLType* out) {
    if (argc != 1) return false;
    if (out) {
        out->state = GMLTypeState::String;
        if (argv[0].state == GMLTypeState::String) {
            out->sVal = argv[0].sVal;
        }
        else {
            std::stringstream ss;
            ss.precision(_round(argv[0].dVal) == argv[0].dVal ? 0 : 2);
            ss << std::fixed << argv[0].dVal;
            out->sVal = ss.str();
        }
    }
    return true;
}

bool Runtime::string_width(unsigned int argc, GMLType* argv, GMLType* out) {
    if (argc != 1) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        if (argv[0].state != GMLTypeState::String) {
            out->dVal = GMLTrue;  // GML default
            return true;
        }

        Font* font = AssetManager::GetFont(_drawFont);
        if (!font->exists) {
            // Default font not sure what to do here
            out->dVal = GMLFalse;
            return true;
        }

        unsigned int longestLine = 1;
        unsigned int curLength = 0;
        for (const char* pC = argv[0].sVal.c_str(); (*pC) != '\0'; pC++) {
            const char c = *pC;
            if (c == '#' && (pC == argv[0].sVal || *(pC - 1) != '\\')) {
                if (curLength > longestLine) longestLine = curLength;
                curLength = 0;
                continue;
            }
            curLength += font->dmap[(c * 6) + 4];
        }

        if (curLength > longestLine) longestLine = curLength;
        out->dVal = longestLine;
    }
    return true;
}

bool Runtime::string_height(unsigned int argc, GMLType* argv, GMLType* out) {
    if (argc != 1) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        if (argv[0].state != GMLTypeState::String) {
            out->dVal = GMLTrue;  // GML default
            return true;
        }

        Font* font = AssetManager::GetFont(_drawFont);
        if (!font->exists) {
            // Default font not sure what to do here
            out->dVal = GMLFalse;
            return true;
        }

        unsigned tallest = 1;
        unsigned int lines = 1;
        for (const char* pC = argv[0].sVal.c_str(); (*pC) != '\0'; pC++) {
            const char c = *pC;
            if (c == '#' && (pC == argv[0].sVal || *(pC - 1) != '\\')) {
                lines++;
                continue;
            }
            unsigned int h = font->dmap[(c * 6) + 3];
            if (h > tallest) tallest = h;
        }

        out->dVal = tallest * lines;
    }
    return true;
}

bool Runtime::tan(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::Double)) return false;
    if (out) {
        out->state = GMLTypeState::Double;
        out->dVal = ::tan(argv[0].dVal);
    }
    return true;
}

bool Runtime::window_set_caption(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!_assertArgs(argc, argv, 1, true, GMLTypeState::String)) return false;
    RSetGameWindowTitle(argv->sVal.c_str());
    return true;
}

bool Runtime::window_get_caption(unsigned int argc, GMLType* argv, GMLType* out) {
    if (argc != 0) return false;
    if (out) {
        out->state = GMLTypeState::String;
        out->sVal = std::string(GetGlobals()->room_caption);
    }
    return true;
}

constexpr bool CRErrorOnUnimplemented = false;
bool Runtime::unimplemented(unsigned int argc, GMLType* argv, GMLType* out) {
    if (!CRErrorOnUnimplemented) {
        if (out) {
            out->state = GMLTypeState::Double;
            out->dVal = GMLFalse;
        }
        return true;
    }
    return false;
}
