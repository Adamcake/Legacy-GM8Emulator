#include <pch.h>

#include "AssetManager.hpp"
#include "CRRuntime.hpp"
#include "Compiled.hpp"
#include "Interpreter.hpp"

namespace GM8Emulator {
    namespace Compiler {
        bool _InterpretLine(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretVar(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretIfElse(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretWith(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretRepeat(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretDoUntil(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretWhile(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretFor(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretSwitch(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretBreak(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretContinue(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretReturn(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretExit(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretAssignment(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretFunction(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);
        bool _InterpretBraces(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos);

        bool _TokenHasValue(const GM8Emulator::Compiler::Token& token, GM8Emulator::Compiler::KeywordType t);
        bool _TokenHasValue(const GM8Emulator::Compiler::Token& token, GM8Emulator::Compiler::OperatorType t);
        bool _TokenHasValue(const GM8Emulator::Compiler::Token& token, GM8Emulator::Compiler::SeparatorType t);
        bool _TokenHasValue(const GM8Emulator::Compiler::Token& token, double t);
        bool _TokenHasValue(const GM8Emulator::Compiler::Token& token, std::string t);
        void _SkipSemicolon(const GM8Emulator::Compiler::TokenList& list, unsigned int& pos);

        bool _IsAsset(std::string_view& name, unsigned int* index = nullptr);
        bool _IsGMLConst(std::string_view& name, double* value = nullptr);
        bool _IsGameValue(std::string_view& name, CRGameVar* value = nullptr);
        bool _IsInstanceVar(std::string_view& name, CRInstanceVar* value = nullptr);

        enum VarType { VARTYPE_INSTANCE, VARTYPE_FIELD, VARTYPE_GAME };
        VarType _getVarType(std::string_view& name, unsigned int* index = nullptr);

        std::vector<std::string> _fieldNames;
        unsigned int _RegisterField(const std::string_view& name);

        std::vector<const char*> _gameValueNames;
        std::vector<const char*> _instanceVarNames;
        std::vector<const char*> _internalFuncNames;
        std::vector<bool (*)(unsigned int, GMLType*, GMLType*)> _gmlFuncs;
        std::vector<char> _operatorPrecedence;
        std::map<const char*, int> _gmlConsts = {{"ANSI_CHARSET", 0}, {"ARABIC_CHARSET", 178}, {"BALTIC_CHARSET", 186}, {"CHINESEBIG5_CHARSET", 136}, {"DEFAULT_CHARSET", 1},
            {"EASTEUROPE_CHARSET", 238}, {"GB2312_CHARSET", 134}, {"GREEK_CHARSET", 161}, {"HANGEUL_CHARSET", 129}, {"HEBREW_CHARSET", 177}, {"JOHAB_CHARSET", 130}, {"MAC_CHARSET", 77},
            {"OEM_CHARSET", 255}, {"RUSSIAN_CHARSET", 204}, {"SHIFTJIS_CHARSET", 128}, {"SYMBOL_CHARSET", 2}, {"THAI_CHARSET", 222}, {"TURKISH_CHARSET", 162}, {"VIETNAMESE_CHARSET", 163}, {"all", -3},
            {"bm_add", 1}, {"bm_dest_alpha", 7}, {"bm_dest_color", 9}, {"bm_inv_dest_alpha", 8}, {"bm_inv_dest_color", 10}, {"bm_inv_src_alpha", 6}, {"bm_inv_src_color", 4}, {"bm_max", 2},
            {"bm_normal", 0}, {"bm_one", 2}, {"bm_src_alpha", 5}, {"bm_src_alpha_sat", 11}, {"bm_src_color", 3}, {"bm_subtract", 3}, {"bm_zero", 1}, {"button_type", 1}, {"c_aqua", 16776960},
            {"c_black", 0}, {"c_blue", 16711680}, {"c_dkgray", 4210752}, {"c_fuchsia", 16711935}, {"c_gray", 8421504}, {"c_green", 32768}, {"c_lime", 65280}, {"c_ltgray", 12632256}, {"c_maroon", 128},
            {"c_navy", 8388608}, {"c_olive", 32896}, {"c_orange", 4235519}, {"c_purple", 8388736}, {"c_red", 255}, {"c_silver", 12632256}, {"c_teal", 8421376}, {"c_white", 16777215},
            {"c_yellow", 65535}, {"cr_appstart", -19}, {"cr_arrow", -2}, {"cr_beam", -4}, {"cr_cross", -3}, {"cr_default", 0}, {"cr_drag", -12}, {"cr_handpoint", -21}, {"cr_help", -20},
            {"cr_hourglass", -11}, {"cr_hsplit", -14}, {"cr_multidrag", -16}, {"cr_no", -18}, {"cr_nodrop", -13}, {"cr_none", -1}, {"cr_size_all", -22}, {"cr_size_nesw", -6}, {"cr_size_ns", -7},
            {"cr_size_nwse", -8}, {"cr_size_we", -9}, {"cr_sqlwait", -17}, {"cr_uparrow", -10}, {"cr_vsplit", -15}, {"device_ios_ipad", 2}, {"device_ios_iphone", 0}, {"device_ios_iphone_retina", 1},
            {"dll_cdecl", 0}, {"dll_stdcall", 1}, {"ef_cloud", 9}, {"ef_ellipse", 2}, {"ef_explosion", 0}, {"ef_firework", 3}, {"ef_flare", 8}, {"ef_rain", 10}, {"ef_ring", 1}, {"ef_smoke", 4},
            {"ef_smokeup", 5}, {"ef_snow", 11}, {"ef_spark", 7}, {"ef_star", 6}, {"ev_alarm", 2}, {"ev_animation_end", 7}, {"ev_boundary", 1}, {"ev_close_button", 30}, {"ev_collision", 4},
            {"ev_create", 0}, {"ev_destroy", 1}, {"ev_draw", 8}, {"ev_end_of_path", 8}, {"ev_game_end", 3}, {"ev_game_start", 2}, {"ev_global_left_button", 50}, {"ev_global_left_press", 53},
            {"ev_global_left_release", 56}, {"ev_global_middle_button", 52}, {"ev_global_middle_press", 55}, {"ev_global_middle_release", 58}, {"ev_global_press", 12}, {"ev_global_release", 13},
            {"ev_global_right_button", 51}, {"ev_global_right_press", 54}, {"ev_global_right_release", 57}, {"ev_joystick1_button1", 21}, {"ev_joystick1_button2", 22}, {"ev_joystick1_button3", 23},
            {"ev_joystick1_button4", 24}, {"ev_joystick1_button5", 25}, {"ev_joystick1_button6", 26}, {"ev_joystick1_button7", 27}, {"ev_joystick1_button8", 28}, {"ev_joystick1_down", 19},
            {"ev_joystick1_left", 16}, {"ev_joystick1_right", 17}, {"ev_joystick1_up", 18}, {"ev_joystick2_button1", 36}, {"ev_joystick2_button2", 37}, {"ev_joystick2_button3", 38},
            {"ev_joystick2_button4", 39}, {"ev_joystick2_button5", 40}, {"ev_joystick2_button6", 41}, {"ev_joystick2_button7", 42}, {"ev_joystick2_button8", 43}, {"ev_joystick2_down", 34},
            {"ev_joystick2_left", 31}, {"ev_joystick2_right", 32}, {"ev_joystick2_up", 33}, {"ev_keyboard", 5}, {"ev_keypress", 9}, {"ev_keyrelease", 10}, {"ev_left_button", 0}, {"ev_left_press", 4},
            {"ev_left_release", 7}, {"ev_middle_button", 2}, {"ev_middle_press", 6}, {"ev_middle_release", 9}, {"ev_mouse", 6}, {"ev_mouse_enter", 10}, {"ev_mouse_leave", 11},
            {"ev_mouse_wheel_down", 61}, {"ev_mouse_wheel_up", 60}, {"ev_no_button", 3}, {"ev_no_more_health", 9}, {"ev_no_more_lives", 6}, {"ev_other", 7}, {"ev_outside", 0}, {"ev_right_button", 1},
            {"ev_right_press", 5}, {"ev_right_release", 8}, {"ev_room_end", 5}, {"ev_room_start", 4}, {"ev_step", 3}, {"ev_step_begin", 1}, {"ev_step_end", 2}, {"ev_step_normal", 0},
            {"ev_trigger", 11}, {"ev_user0", 10}, {"ev_user1", 11}, {"ev_user10", 20}, {"ev_user11", 21}, {"ev_user12", 22}, {"ev_user13", 23}, {"ev_user14", 24}, {"ev_user15", 25}, {"ev_user2", 12},
            {"ev_user3", 13}, {"ev_user4", 14}, {"ev_user5", 15}, {"ev_user6", 16}, {"ev_user7", 17}, {"ev_user8", 18}, {"ev_user9", 19}, {"fa_archive", 32}, {"fa_bottom", 2}, {"fa_center", 1},
            {"fa_directory", 16}, {"fa_hidden", 2}, {"fa_left", 0}, {"fa_middle", 1}, {"fa_readonly", 1}, {"fa_right", 2}, {"fa_sysfile", 4}, {"fa_top", 0}, {"fa_volumeid", 8}, {"false", 0},
            {"global", -5}, {"local", -7}, {"mb_any", -1}, {"mb_left", 1}, {"mb_middle", 3}, {"mb_none", 0}, {"mb_right", 2}, {"noone", -4}, {"os_android", 5}, {"os_ios", 4}, {"os_macosx", 2},
            {"os_psp", 3}, {"os_win32", 0}, {"other", -2}, {"pr_linelist", 2}, {"pr_linestrip", 3}, {"pr_pointlist", 1}, {"pr_trianglefan", 6}, {"pr_trianglelist", 4}, {"pr_trianglestrip", 5},
            {"ps_change_all", 0}, {"ps_change_motion", 2}, {"ps_change_shape", 1}, {"ps_deflect_horizontal", 1}, {"ps_deflect_vertical", 0}, {"ps_distr_gaussian", 1}, {"ps_distr_invgaussian", 2},
            {"ps_distr_linear", 0}, {"ps_force_constant", 0}, {"ps_force_linear", 1}, {"ps_force_quadratic", 2}, {"ps_shape_diamond", 2}, {"ps_shape_ellipse", 1}, {"ps_shape_line", 3},
            {"ps_shape_rectangle", 0}, {"pt_shape_circle", 5}, {"pt_shape_cloud", 11}, {"pt_shape_disk", 1}, {"pt_shape_explosion", 10}, {"pt_shape_flare", 8}, {"pt_shape_line", 3},
            {"pt_shape_pixel", 0}, {"pt_shape_ring", 6}, {"pt_shape_smoke", 12}, {"pt_shape_snow", 13}, {"pt_shape_spark", 9}, {"pt_shape_sphere", 7}, {"pt_shape_square", 2}, {"pt_shape_star", 4},
            {"se_chorus", 1}, {"se_compressor", 32}, {"se_echo", 2}, {"se_equalizer", 64}, {"se_flanger", 4}, {"se_gargle", 8}, {"se_none", 0}, {"se_reverb", 16}, {"self", -1}, {"text_type", 0},
            {"true", 1}, {"ty_real", 0}, {"ty_string", 1}, {"vk_add", 107}, {"vk_alt", 18}, {"vk_anykey", 1}, {"vk_backspace", 8}, {"vk_control", 17}, {"vk_decimal", 110}, {"vk_delete", 46},
            {"vk_divide", 111}, {"vk_down", 40}, {"vk_end", 35}, {"vk_enter", 13}, {"vk_escape", 27}, {"vk_f1", 112}, {"vk_f10", 121}, {"vk_f11", 122}, {"vk_f12", 123}, {"vk_f2", 113}, {"vk_f3", 114},
            {"vk_f4", 115}, {"vk_f5", 116}, {"vk_f6", 117}, {"vk_f7", 118}, {"vk_f8", 119}, {"vk_f9", 120}, {"vk_home", 36}, {"vk_insert", 45}, {"vk_lalt", 164}, {"vk_lcontrol", 162}, {"vk_left", 37},
            {"vk_lshift", 160}, {"vk_multiply", 106}, {"vk_nokey", 0}, {"vk_numpad0", 96}, {"vk_numpad1", 97}, {"vk_numpad2", 98}, {"vk_numpad3", 99}, {"vk_numpad4", 100}, {"vk_numpad5", 101},
            {"vk_numpad6", 102}, {"vk_numpad7", 103}, {"vk_numpad8", 104}, {"vk_numpad9", 105}, {"vk_pagedown", 34}, {"vk_pageup", 33}, {"vk_pause", 19}, {"vk_printscreen", 44}, {"vk_ralt", 165},
            {"vk_rcontrol", 163}, {"vk_return", 13}, {"vk_right", 39}, {"vk_rshift", 161}, {"vk_shift", 16}, {"vk_space", 32}, {"vk_subtract", 109}, {"vk_tab", 9}, {"vk_up", 38}};
    }
}

bool GM8Emulator::Compiler::Init(GlobalValues* globals) {
    for (unsigned int op = 0; op < 20; op++) {
        switch (op) {
            case OPERATOR_DIVIDE:
            case OPERATOR_MULTIPLY:
            case OPERATOR_MOD:
            case OPERATOR_DIV:
                _operatorPrecedence.push_back(5);
                break;
            case OPERATOR_ADD:
            case OPERATOR_SUBTRACT:
                _operatorPrecedence.push_back(4);
                break;
            case OPERATOR_LSHIFT:
            case OPERATOR_RSHIFT:
                _operatorPrecedence.push_back(3);
                break;
            case OPERATOR_BITWISE_AND:
            case OPERATOR_BITWISE_OR:
            case OPERATOR_BITWISE_XOR:
                _operatorPrecedence.push_back(2);
                break;
            case OPERATOR_EQUALS:
            case OPERATOR_NOT_EQUAL:
            case OPERATOR_LT:
            case OPERATOR_LTE:
            case OPERATOR_GT:
            case OPERATOR_GTE:
                _operatorPrecedence.push_back(1);
                break;
            case OPERATOR_BOOLEAN_AND:
            case OPERATOR_BOOLEAN_OR:
            case OPERATOR_BOOLEAN_XOR:
                _operatorPrecedence.push_back(0);
                break;
            default:
                return false;
        }
    }

    _gameValueNames.reserve(_GAME_VALUE_COUNT);
    for (unsigned int var = 0; var < _GAME_VALUE_COUNT; var++) {
        switch (var) {
            case ARGUMENT:
                _gameValueNames.push_back("argument");
                break;
            case ARGUMENT0:
                _gameValueNames.push_back("argument0");
                break;
            case ARGUMENT1:
                _gameValueNames.push_back("argument1");
                break;
            case ARGUMENT2:
                _gameValueNames.push_back("argument2");
                break;
            case ARGUMENT3:
                _gameValueNames.push_back("argument3");
                break;
            case ARGUMENT4:
                _gameValueNames.push_back("argument4");
                break;
            case ARGUMENT5:
                _gameValueNames.push_back("argument5");
                break;
            case ARGUMENT6:
                _gameValueNames.push_back("argument6");
                break;
            case ARGUMENT7:
                _gameValueNames.push_back("argument7");
                break;
            case ARGUMENT8:
                _gameValueNames.push_back("argument8");
                break;
            case ARGUMENT9:
                _gameValueNames.push_back("argument9");
                break;
            case ARGUMENT10:
                _gameValueNames.push_back("argument10");
                break;
            case ARGUMENT11:
                _gameValueNames.push_back("argument11");
                break;
            case ARGUMENT12:
                _gameValueNames.push_back("argument12");
                break;
            case ARGUMENT13:
                _gameValueNames.push_back("argument13");
                break;
            case ARGUMENT14:
                _gameValueNames.push_back("argument14");
                break;
            case ARGUMENT15:
                _gameValueNames.push_back("argument15");
                break;
            case HEALTH:
                _gameValueNames.push_back("health");
                break;
            case INSTANCE_COUNT:
                _gameValueNames.push_back("instance_count");
                break;
            case LIVES:
                _gameValueNames.push_back("lives");
                break;
            case MOUSE_X:
                _gameValueNames.push_back("mouse_x");
                break;
            case MOUSE_Y:
                _gameValueNames.push_back("mouse_y");
                break;
            case ROOM:
                _gameValueNames.push_back("room");
                break;
            case ROOM_SPEED:
                _gameValueNames.push_back("room_speed");
                break;
            case ROOM_WIDTH:
                _gameValueNames.push_back("room_width");
                break;
            case ROOM_HEIGHT:
                _gameValueNames.push_back("room_height");
                break;
            case ROOM_CAPTION:
                _gameValueNames.push_back("room_caption");
                break;
            default:
                // There's something in the enum that isn't listed here. Abort
                return false;
        }
    }

    _instanceVarNames.reserve(_INSTANCE_VAR_COUNT);
    for (unsigned int var = 0; var < _INSTANCE_VAR_COUNT; var++) {
        switch (var) {
            case IV_INSTANCE_ID:
                _instanceVarNames.push_back("id");
                break;
            case IV_OBJECT_INDEX:
                _instanceVarNames.push_back("object_index");
                break;
            case IV_SOLID:
                _instanceVarNames.push_back("solid");
                break;
            case IV_VISIBLE:
                _instanceVarNames.push_back("visible");
                break;
            case IV_PERSISTENT:
                _instanceVarNames.push_back("persistent");
                break;
            case IV_DEPTH:
                _instanceVarNames.push_back("depth");
                break;
            case IV_ALARM:
                _instanceVarNames.push_back("alarm");
                break;
            case IV_SPRITE_INDEX:
                _instanceVarNames.push_back("sprite_index");
                break;
            case IV_IMAGE_ALPHA:
                _instanceVarNames.push_back("image_alpha");
                break;
            case IV_IMAGE_BLEND:
                _instanceVarNames.push_back("image_blend");
                break;
            case IV_IMAGE_INDEX:
                _instanceVarNames.push_back("image_index");
                break;
            case IV_IMAGE_SPEED:
                _instanceVarNames.push_back("image_speed");
                break;
            case IV_IMAGE_XSCALE:
                _instanceVarNames.push_back("image_xscale");
                break;
            case IV_IMAGE_YSCALE:
                _instanceVarNames.push_back("image_yscale");
                break;
            case IV_IMAGE_ANGLE:
                _instanceVarNames.push_back("image_angle");
                break;
            case IV_MASK_INDEX:
                _instanceVarNames.push_back("mask_index");
                break;
            case IV_DIRECTION:
                _instanceVarNames.push_back("direction");
                break;
            case IV_FRICTION:
                _instanceVarNames.push_back("friction");
                break;
            case IV_GRAVITY:
                _instanceVarNames.push_back("gravity");
                break;
            case IV_GRAVITY_DIRECTION:
                _instanceVarNames.push_back("gravity_direction");
                break;
            case IV_HSPEED:
                _instanceVarNames.push_back("hspeed");
                break;
            case IV_VSPEED:
                _instanceVarNames.push_back("vspeed");
                break;
            case IV_SPEED:
                _instanceVarNames.push_back("speed");
                break;
            case IV_X:
                _instanceVarNames.push_back("x");
                break;
            case IV_Y:
                _instanceVarNames.push_back("y");
                break;
            case IV_XPREVIOUS:
                _instanceVarNames.push_back("xprevious");
                break;
            case IV_YPREVIOUS:
                _instanceVarNames.push_back("yprevious");
                break;
            case IV_XSTART:
                _instanceVarNames.push_back("xstart");
                break;
            case IV_YSTART:
                _instanceVarNames.push_back("ystart");
                break;
            case IV_PATH_INDEX:
                _instanceVarNames.push_back("path_index");
                break;
            case IV_PATH_POSITION:
                _instanceVarNames.push_back("path_position");
                break;
            case IV_PATH_POSITIONPREVIOUS:
                _instanceVarNames.push_back("path_positionprevious");
                break;
            case IV_PATH_SPEED:
                _instanceVarNames.push_back("path_speed");
                break;
            case IV_PATH_SCALE:
                _instanceVarNames.push_back("path_scale");
                break;
            case IV_PATH_ORIENTATION:
                _instanceVarNames.push_back("path_orientation");
                break;
            case IV_PATH_ENDACTION:
                _instanceVarNames.push_back("path_endaction");
                break;
            case IV_TIMELINE_INDEX:
                _instanceVarNames.push_back("timeline_index");
                break;
            case IV_TIMELINE_RUNNING:
                _instanceVarNames.push_back("timeline_running");
                break;
            case IV_TIMELINE_SPEED:
                _instanceVarNames.push_back("timeline_speed");
                break;
            case IV_TIMELINE_POSITION:
                _instanceVarNames.push_back("timeline_position");
                break;
            case IV_TIMELINE_LOOP:
                _instanceVarNames.push_back("timeline_loop");
                break;

            case IV_SPRITE_WIDTH:
                _instanceVarNames.push_back("sprite_width");
                break;
            case IV_SPRITE_HEIGHT:
                _instanceVarNames.push_back("sprite_height");
                break;
            case IV_BBOX_LEFT:
                _instanceVarNames.push_back("bbox_left");
                break;
            case IV_BBOX_RIGHT:
                _instanceVarNames.push_back("bbox_right");
                break;
            case IV_BBOX_BOTTOM:
                _instanceVarNames.push_back("bbox_bottom");
                break;
            case IV_BBOX_TOP:
                _instanceVarNames.push_back("bbox_top");
                break;
            default:
                // Something in the enum isn't listed here
                return false;
        }
    }
    _internalFuncNames.reserve(_INTERNAL_FUNC_COUNT);
    for (unsigned int func = 0; func < _INTERNAL_FUNC_COUNT; func++) {
        switch (func) {
            case ABS:
                _internalFuncNames.push_back("abs");
                _gmlFuncs.push_back(&Runtime::abs);
                break;
            case ANSI_CHAR:
                _internalFuncNames.push_back("ansi_char");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ARCCOS:
                _internalFuncNames.push_back("arccos");
                _gmlFuncs.push_back(&Runtime::arccos);
                break;
            case ARCSIN:
                _internalFuncNames.push_back("arcsin");
                _gmlFuncs.push_back(&Runtime::arcsin);
                break;
            case ARCTAN:
                _internalFuncNames.push_back("arctan");
                _gmlFuncs.push_back(&Runtime::arctan);
                break;
            case ARCTAN2:
                _internalFuncNames.push_back("arctan2");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_ADD:
                _internalFuncNames.push_back("background_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_ADD_BACKGROUND:
                _internalFuncNames.push_back("background_add_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_ASSIGN:
                _internalFuncNames.push_back("background_assign");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_CREATE_COLOR:
                _internalFuncNames.push_back("background_create_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_CREATE_FROM_SCREEN:
                _internalFuncNames.push_back("background_create_from_screen");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_CREATE_FROM_SURFACE:
                _internalFuncNames.push_back("background_create_from_surface");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_CREATE_GRADIENT:
                _internalFuncNames.push_back("background_create_gradient");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_DELETE:
                _internalFuncNames.push_back("background_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_DUPLICATE:
                _internalFuncNames.push_back("background_duplicate");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_EXISTS:
                _internalFuncNames.push_back("background_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_GET_HEIGHT:
                _internalFuncNames.push_back("background_get_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_GET_NAME:
                _internalFuncNames.push_back("background_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_GET_TEXTURE:
                _internalFuncNames.push_back("background_get_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_GET_WIDTH:
                _internalFuncNames.push_back("background_get_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_REPLACE:
                _internalFuncNames.push_back("background_replace");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_REPLACE_BACKGROUND:
                _internalFuncNames.push_back("background_replace_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_SAVE:
                _internalFuncNames.push_back("background_save");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case BACKGROUND_SET_ALPHA_FROM_BACKGROUND:
                _internalFuncNames.push_back("background_set_alpha_from_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_CLOSE_DOOR:
                _internalFuncNames.push_back("cd_close_door");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_INIT:
                _internalFuncNames.push_back("cd_init");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_LENGTH:
                _internalFuncNames.push_back("cd_length");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_NUMBER:
                _internalFuncNames.push_back("cd_number");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_OPEN_DOOR:
                _internalFuncNames.push_back("cd_open_door");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_PAUSE:
                _internalFuncNames.push_back("cd_pause");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_PAUSED:
                _internalFuncNames.push_back("cd_paused");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_PLAY:
                _internalFuncNames.push_back("cd_play");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_PLAYING:
                _internalFuncNames.push_back("cd_playing");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_POSITION:
                _internalFuncNames.push_back("cd_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_PRESENT:
                _internalFuncNames.push_back("cd_present");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_RESUME:
                _internalFuncNames.push_back("cd_resume");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_SET_POSITION:
                _internalFuncNames.push_back("cd_set_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_SET_TRACK_POSITION:
                _internalFuncNames.push_back("cd_set_track_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_STOP:
                _internalFuncNames.push_back("cd_stop");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_TRACK:
                _internalFuncNames.push_back("cd_track");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_TRACK_LENGTH:
                _internalFuncNames.push_back("cd_track_length");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CD_TRACK_POSITION:
                _internalFuncNames.push_back("cd_track_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CEIL:
                _internalFuncNames.push_back("ceil");
                _gmlFuncs.push_back(&Runtime::ceil);
                break;
            case CHOOSE:
                _internalFuncNames.push_back("choose");
                _gmlFuncs.push_back(&Runtime::choose);
                break;
            case CHR:
                _internalFuncNames.push_back("chr");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CLIPBOARD_GET_TEXT:
                _internalFuncNames.push_back("clipboard_get_text");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CLIPBOARD_HAS_TEXT:
                _internalFuncNames.push_back("clipboard_has_text");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case CLIPBOARD_SET_TEXT:
                _internalFuncNames.push_back("clipboard_set_text");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLLISION_CIRCLE:
                _internalFuncNames.push_back("collision_circle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLLISION_ELLIPSE:
                _internalFuncNames.push_back("collision_ellipse");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLLISION_LINE:
                _internalFuncNames.push_back("collision_line");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLLISION_POINT:
                _internalFuncNames.push_back("collision_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLLISION_RECTANGLE:
                _internalFuncNames.push_back("collision_rectangle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLOR_GET_BLUE:
                _internalFuncNames.push_back("color_get_blue");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLOR_GET_GREEN:
                _internalFuncNames.push_back("color_get_green");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLOR_GET_HUE:
                _internalFuncNames.push_back("color_get_hue");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLOR_GET_RED:
                _internalFuncNames.push_back("color_get_red");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLOR_GET_SATURATION:
                _internalFuncNames.push_back("color_get_saturation");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COLOR_GET_VALUE:
                _internalFuncNames.push_back("color_get_value");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case COS:
                _internalFuncNames.push_back("cos");
                _gmlFuncs.push_back(&Runtime::cos);
                break;
            case D3D_DRAW_BLOCK:
                _internalFuncNames.push_back("d3d_draw_block");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_DRAW_CONE:
                _internalFuncNames.push_back("d3d_draw_cone");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_DRAW_CYLINDER:
                _internalFuncNames.push_back("d3d_draw_cylinder");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_DRAW_ELLIPSOID:
                _internalFuncNames.push_back("d3d_draw_ellipsoid");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_DRAW_FLOOR:
                _internalFuncNames.push_back("d3d_draw_floor");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_DRAW_WALL:
                _internalFuncNames.push_back("d3d_draw_wall");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_END:
                _internalFuncNames.push_back("d3d_end");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_LIGHT_DEFINE_AMBIENT:
                _internalFuncNames.push_back("d3d_light_define_ambient");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_LIGHT_DEFINE_DIRECTION:
                _internalFuncNames.push_back("d3d_light_define_direction");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_LIGHT_DEFINE_POINT:
                _internalFuncNames.push_back("d3d_light_define_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_LIGHT_ENABLE:
                _internalFuncNames.push_back("d3d_light_enable");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_BLOCK:
                _internalFuncNames.push_back("d3d_model_block");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_CLEAR:
                _internalFuncNames.push_back("d3d_model_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_CONE:
                _internalFuncNames.push_back("d3d_model_cone");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_CREATE:
                _internalFuncNames.push_back("d3d_model_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_CYLINDER:
                _internalFuncNames.push_back("d3d_model_cylinder");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_DESTROY:
                _internalFuncNames.push_back("d3d_model_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_DRAW:
                _internalFuncNames.push_back("d3d_model_draw");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_ELLIPSOID:
                _internalFuncNames.push_back("d3d_model_ellipsoid");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_FLOOR:
                _internalFuncNames.push_back("d3d_model_floor");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_LOAD:
                _internalFuncNames.push_back("d3d_model_load");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_PRIMITIVE_BEGIN:
                _internalFuncNames.push_back("d3d_model_primitive_begin");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_PRIMITIVE_END:
                _internalFuncNames.push_back("d3d_model_primitive_end");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_SAVE:
                _internalFuncNames.push_back("d3d_model_save");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX:
                _internalFuncNames.push_back("d3d_model_vertex");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX_COLOR:
                _internalFuncNames.push_back("d3d_model_vertex_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX_NORMAL:
                _internalFuncNames.push_back("d3d_model_vertex_normal");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX_NORMAL_COLOR:
                _internalFuncNames.push_back("d3d_model_vertex_normal_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX_NORMAL_TEXTURE:
                _internalFuncNames.push_back("d3d_model_vertex_normal_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX_NORMAL_TEXTURE_COLOR:
                _internalFuncNames.push_back("d3d_model_vertex_normal_texture_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX_TEXTURE:
                _internalFuncNames.push_back("d3d_model_vertex_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_VERTEX_TEXTURE_COLOR:
                _internalFuncNames.push_back("d3d_model_vertex_texture_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_MODEL_WALL:
                _internalFuncNames.push_back("d3d_model_wall");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_PRIMITIVE_BEGIN:
                _internalFuncNames.push_back("d3d_primitive_begin");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_PRIMITIVE_BEGIN_TEXTURE:
                _internalFuncNames.push_back("d3d_primitive_begin_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_PRIMITIVE_END:
                _internalFuncNames.push_back("d3d_primitive_end");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_CULLING:
                _internalFuncNames.push_back("d3d_set_culling");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_DEPTH:
                _internalFuncNames.push_back("d3d_set_depth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_FOG:
                _internalFuncNames.push_back("d3d_set_fog");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_HIDDEN:
                _internalFuncNames.push_back("d3d_set_hidden");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_LIGHTING:
                _internalFuncNames.push_back("d3d_set_lighting");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_PERSPECTIVE:
                _internalFuncNames.push_back("d3d_set_perspective");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_PROJECTION:
                _internalFuncNames.push_back("d3d_set_projection");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_PROJECTION_EXT:
                _internalFuncNames.push_back("d3d_set_projection_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_PROJECTION_ORTHO:
                _internalFuncNames.push_back("d3d_set_projection_ortho");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_PROJECTION_PERSPECTIVE:
                _internalFuncNames.push_back("d3d_set_projection_perspective");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_SET_SHADING:
                _internalFuncNames.push_back("d3d_set_shading");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_START:
                _internalFuncNames.push_back("d3d_start");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_ADD_ROTATION_AXIS:
                _internalFuncNames.push_back("d3d_transform_add_rotation_axis");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_ADD_ROTATION_X:
                _internalFuncNames.push_back("d3d_transform_add_rotation_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_ADD_ROTATION_Y:
                _internalFuncNames.push_back("d3d_transform_add_rotation_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_ADD_ROTATION_Z:
                _internalFuncNames.push_back("d3d_transform_add_rotation_z");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_ADD_SCALING:
                _internalFuncNames.push_back("d3d_transform_add_scaling");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_ADD_TRANSLATION:
                _internalFuncNames.push_back("d3d_transform_add_translation");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_SET_IDENTITY:
                _internalFuncNames.push_back("d3d_transform_set_identity");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_SET_ROTATION_AXIS:
                _internalFuncNames.push_back("d3d_transform_set_rotation_axis");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_SET_ROTATION_X:
                _internalFuncNames.push_back("d3d_transform_set_rotation_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_SET_ROTATION_Y:
                _internalFuncNames.push_back("d3d_transform_set_rotation_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_SET_ROTATION_Z:
                _internalFuncNames.push_back("d3d_transform_set_rotation_z");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_SET_SCALING:
                _internalFuncNames.push_back("d3d_transform_set_scaling");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_SET_TRANSLATION:
                _internalFuncNames.push_back("d3d_transform_set_translation");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_STACK_CLEAR:
                _internalFuncNames.push_back("d3d_transform_stack_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_STACK_DISCARD:
                _internalFuncNames.push_back("d3d_transform_stack_discard");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_STACK_EMPTY:
                _internalFuncNames.push_back("d3d_transform_stack_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_STACK_POP:
                _internalFuncNames.push_back("d3d_transform_stack_pop");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_STACK_PUSH:
                _internalFuncNames.push_back("d3d_transform_stack_push");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_TRANSFORM_STACK_TOP:
                _internalFuncNames.push_back("d3d_transform_stack_top");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX:
                _internalFuncNames.push_back("d3d_vertex");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX_COLOR:
                _internalFuncNames.push_back("d3d_vertex_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX_NORMAL:
                _internalFuncNames.push_back("d3d_vertex_normal");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX_NORMAL_COLOR:
                _internalFuncNames.push_back("d3d_vertex_normal_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX_NORMAL_TEXTURE:
                _internalFuncNames.push_back("d3d_vertex_normal_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX_NORMAL_TEXTURE_COLOR:
                _internalFuncNames.push_back("d3d_vertex_normal_texture_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX_TEXTURE:
                _internalFuncNames.push_back("d3d_vertex_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case D3D_VERTEX_TEXTURE_COLOR:
                _internalFuncNames.push_back("d3d_vertex_texture_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_COMPARE_DATE:
                _internalFuncNames.push_back("date_compare_date");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_COMPARE_DATETIME:
                _internalFuncNames.push_back("date_compare_datetime");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_COMPARE_TIME:
                _internalFuncNames.push_back("date_compare_time");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_CREATE_DATE:
                _internalFuncNames.push_back("date_create_date");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_CREATE_DATETIME:
                _internalFuncNames.push_back("date_create_datetime");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_CREATE_TIME:
                _internalFuncNames.push_back("date_create_time");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_CURRENT_DATE:
                _internalFuncNames.push_back("date_current_date");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_CURRENT_DATETIME:
                _internalFuncNames.push_back("date_current_datetime");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_CURRENT_TIME:
                _internalFuncNames.push_back("date_current_time");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_DATE_OF:
                _internalFuncNames.push_back("date_date_of");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_DATE_STRING:
                _internalFuncNames.push_back("date_date_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_DATETIME_STRING:
                _internalFuncNames.push_back("date_datetime_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_DAY_SPAN:
                _internalFuncNames.push_back("date_day_span");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_DAYS_IN_MONTH:
                _internalFuncNames.push_back("date_days_in_month");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_DAYS_IN_YEAR:
                _internalFuncNames.push_back("date_days_in_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_DAY:
                _internalFuncNames.push_back("date_get_day");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_DAY_OF_YEAR:
                _internalFuncNames.push_back("date_get_day_of_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_HOUR:
                _internalFuncNames.push_back("date_get_hour");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_HOUR_OF_YEAR:
                _internalFuncNames.push_back("date_get_hour_of_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_MINUTE:
                _internalFuncNames.push_back("date_get_minute");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_MINUTE_OF_YEAR:
                _internalFuncNames.push_back("date_get_minute_of_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_MONTH:
                _internalFuncNames.push_back("date_get_month");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_SECOND:
                _internalFuncNames.push_back("date_get_second");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_SECOND_OF_YEAR:
                _internalFuncNames.push_back("date_get_second_of_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_WEEK:
                _internalFuncNames.push_back("date_get_week");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_WEEKDAY:
                _internalFuncNames.push_back("date_get_weekday");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_GET_YEAR:
                _internalFuncNames.push_back("date_get_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_HOUR_SPAN:
                _internalFuncNames.push_back("date_hour_span");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_INC_DAY:
                _internalFuncNames.push_back("date_inc_day");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_INC_HOUR:
                _internalFuncNames.push_back("date_inc_hour");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_INC_MINUTE:
                _internalFuncNames.push_back("date_inc_minute");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_INC_MONTH:
                _internalFuncNames.push_back("date_inc_month");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_INC_SECOND:
                _internalFuncNames.push_back("date_inc_second");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_INC_WEEK:
                _internalFuncNames.push_back("date_inc_week");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_INC_YEAR:
                _internalFuncNames.push_back("date_inc_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_IS_TODAY:
                _internalFuncNames.push_back("date_is_today");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_LEAP_YEAR:
                _internalFuncNames.push_back("date_leap_year");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_MINUTE_SPAN:
                _internalFuncNames.push_back("date_minute_span");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_MONTH_SPAN:
                _internalFuncNames.push_back("date_month_span");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_SECOND_SPAN:
                _internalFuncNames.push_back("date_second_span");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_TIME_OF:
                _internalFuncNames.push_back("date_time_of");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_TIME_STRING:
                _internalFuncNames.push_back("date_time_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_VALID_DATE:
                _internalFuncNames.push_back("date_valid_date");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_VALID_DATETIME:
                _internalFuncNames.push_back("date_valid_datetime");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_VALID_TIME:
                _internalFuncNames.push_back("date_valid_time");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_WEEK_SPAN:
                _internalFuncNames.push_back("date_week_span");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DATE_YEAR_SPAN:
                _internalFuncNames.push_back("date_year_span");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DEGTORAD:
                _internalFuncNames.push_back("degtorad");
                _gmlFuncs.push_back(&Runtime::degtorad);
                break;
            case DIRECTORY_CREATE:
                _internalFuncNames.push_back("directory_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DIRECTORY_EXISTS:
                _internalFuncNames.push_back("directory_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISCARD_INCLUDE_FILE:
                _internalFuncNames.push_back("discard_include_file");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISK_FREE:
                _internalFuncNames.push_back("disk_free");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISK_SIZE:
                _internalFuncNames.push_back("disk_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_GET_COLORDEPTH:
                _internalFuncNames.push_back("display_get_colordepth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_GET_FREQUENCY:
                _internalFuncNames.push_back("display_get_frequency");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_GET_HEIGHT:
                _internalFuncNames.push_back("display_get_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_GET_WIDTH:
                _internalFuncNames.push_back("display_get_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_MOUSE_GET_X:
                _internalFuncNames.push_back("display_mouse_get_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_MOUSE_GET_Y:
                _internalFuncNames.push_back("display_mouse_get_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_MOUSE_SET:
                _internalFuncNames.push_back("display_mouse_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_RESET:
                _internalFuncNames.push_back("display_reset");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_SET_ALL:
                _internalFuncNames.push_back("display_set_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_SET_COLORDEPTH:
                _internalFuncNames.push_back("display_set_colordepth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_SET_FREQUENCY:
                _internalFuncNames.push_back("display_set_frequency");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_SET_SIZE:
                _internalFuncNames.push_back("display_set_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISPLAY_TEST_ALL:
                _internalFuncNames.push_back("display_test_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DISTANCE_TO_OBJECT:
                _internalFuncNames.push_back("distance_to_object");
                _gmlFuncs.push_back(&Runtime::distance_to_object);
                break;
            case DISTANCE_TO_POINT:
                _internalFuncNames.push_back("distance_to_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DOT_PRODUCT:
                _internalFuncNames.push_back("dot_product");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_ARROW:
                _internalFuncNames.push_back("draw_arrow");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND:
                _internalFuncNames.push_back("draw_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_EXT:
                _internalFuncNames.push_back("draw_background_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_GENERAL:
                _internalFuncNames.push_back("draw_background_general");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_PART:
                _internalFuncNames.push_back("draw_background_part");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_PART_EXT:
                _internalFuncNames.push_back("draw_background_part_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_STRETCHED:
                _internalFuncNames.push_back("draw_background_stretched");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_STRETCHED_EXT:
                _internalFuncNames.push_back("draw_background_stretched_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_TILED:
                _internalFuncNames.push_back("draw_background_tiled");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BACKGROUND_TILED_EXT:
                _internalFuncNames.push_back("draw_background_tiled_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_BUTTON:
                _internalFuncNames.push_back("draw_button");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_CIRCLE:
                _internalFuncNames.push_back("draw_circle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_CIRCLE_COLOR:
                _internalFuncNames.push_back("draw_circle_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_CLEAR:
                _internalFuncNames.push_back("draw_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_CLEAR_ALPHA:
                _internalFuncNames.push_back("draw_clear_alpha");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_ELLIPSE:
                _internalFuncNames.push_back("draw_ellipse");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_ELLIPSE_COLOR:
                _internalFuncNames.push_back("draw_ellipse_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_GET_ALPHA:
                _internalFuncNames.push_back("draw_get_alpha");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_GET_COLOR:
                _internalFuncNames.push_back("draw_get_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_GETPIXEL:
                _internalFuncNames.push_back("draw_getpixel");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_HEALTHBAR:
                _internalFuncNames.push_back("draw_healthbar");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_HIGHSCORE:
                _internalFuncNames.push_back("draw_highscore");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_LINE:
                _internalFuncNames.push_back("draw_line");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_LINE_COLOR:
                _internalFuncNames.push_back("draw_line_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_LINE_WIDTH:
                _internalFuncNames.push_back("draw_line_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_LINE_WIDTH_COLOR:
                _internalFuncNames.push_back("draw_line_width_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_PATH:
                _internalFuncNames.push_back("draw_path");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_POINT:
                _internalFuncNames.push_back("draw_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_POINT_COLOR:
                _internalFuncNames.push_back("draw_point_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_PRIMITIVE_BEGIN:
                _internalFuncNames.push_back("draw_primitive_begin");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_PRIMITIVE_BEGIN_TEXTURE:
                _internalFuncNames.push_back("draw_primitive_begin_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_PRIMITIVE_END:
                _internalFuncNames.push_back("draw_primitive_end");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_RECTANGLE:
                _internalFuncNames.push_back("draw_rectangle");
                _gmlFuncs.push_back(&Runtime::draw_rectangle);
                break;
            case DRAW_RECTANGLE_COLOR:
                _internalFuncNames.push_back("draw_rectangle_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_ROUNDRECT:
                _internalFuncNames.push_back("draw_roundrect");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_ROUNDRECT_COLOR:
                _internalFuncNames.push_back("draw_roundrect_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SELF:
                _internalFuncNames.push_back("draw_self");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SET_ALPHA:
                _internalFuncNames.push_back("draw_set_alpha");
                _gmlFuncs.push_back(&Runtime::draw_set_alpha);
                break;
            case DRAW_SET_BLEND_MODE:
                _internalFuncNames.push_back("draw_set_blend_mode");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SET_BLEND_MODE_EXT:
                _internalFuncNames.push_back("draw_set_blend_mode_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SET_CIRCLE_PRECISION:
                _internalFuncNames.push_back("draw_set_circle_precision");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SET_COLOR:
                _internalFuncNames.push_back("draw_set_color");
                _gmlFuncs.push_back(&Runtime::draw_set_color);
                break;
            case DRAW_SET_FONT:
                _internalFuncNames.push_back("draw_set_font");
                _gmlFuncs.push_back(&Runtime::draw_set_font);
                break;
            case DRAW_SET_HALIGN:
                _internalFuncNames.push_back("draw_set_halign");
                _gmlFuncs.push_back(&Runtime::draw_set_halign);
                break;
            case DRAW_SET_VALIGN:
                _internalFuncNames.push_back("draw_set_valign");
                _gmlFuncs.push_back(&Runtime::draw_set_valign);
                break;
            case DRAW_SPRITE:
                _internalFuncNames.push_back("draw_sprite");
                _gmlFuncs.push_back(&Runtime::draw_sprite);
                break;
            case DRAW_SPRITE_EXT:
                _internalFuncNames.push_back("draw_sprite_ext");
                _gmlFuncs.push_back(&Runtime::draw_sprite_ext);
                break;
            case DRAW_SPRITE_GENERAL:
                _internalFuncNames.push_back("draw_sprite_general");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SPRITE_PART:
                _internalFuncNames.push_back("draw_sprite_part");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SPRITE_PART_EXT:
                _internalFuncNames.push_back("draw_sprite_part_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SPRITE_STRETCHED:
                _internalFuncNames.push_back("draw_sprite_stretched");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SPRITE_STRETCHED_EXT:
                _internalFuncNames.push_back("draw_sprite_stretched_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SPRITE_TILED:
                _internalFuncNames.push_back("draw_sprite_tiled");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SPRITE_TILED_EXT:
                _internalFuncNames.push_back("draw_sprite_tiled_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE:
                _internalFuncNames.push_back("draw_surface");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_EXT:
                _internalFuncNames.push_back("draw_surface_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_GENERAL:
                _internalFuncNames.push_back("draw_surface_general");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_PART:
                _internalFuncNames.push_back("draw_surface_part");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_PART_EXT:
                _internalFuncNames.push_back("draw_surface_part_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_STRETCHED:
                _internalFuncNames.push_back("draw_surface_stretched");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_STRETCHED_EXT:
                _internalFuncNames.push_back("draw_surface_stretched_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_TILED:
                _internalFuncNames.push_back("draw_surface_tiled");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_SURFACE_TILED_EXT:
                _internalFuncNames.push_back("draw_surface_tiled_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TEXT:
                _internalFuncNames.push_back("draw_text");
                _gmlFuncs.push_back(&Runtime::draw_text);
                break;
            case DRAW_TEXT_COLOR:
                _internalFuncNames.push_back("draw_text_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TEXT_EXT:
                _internalFuncNames.push_back("draw_text_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TEXT_EXT_COLOR:
                _internalFuncNames.push_back("draw_text_ext_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TEXT_EXT_TRANSFORMED:
                _internalFuncNames.push_back("draw_text_ext_transformed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TEXT_EXT_TRANSFORMED_COLOR:
                _internalFuncNames.push_back("draw_text_ext_transformed_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TEXT_TRANSFORMED:
                _internalFuncNames.push_back("draw_text_transformed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TEXT_TRANSFORMED_COLOR:
                _internalFuncNames.push_back("draw_text_transformed_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TRIANGLE:
                _internalFuncNames.push_back("draw_triangle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_TRIANGLE_COLOR:
                _internalFuncNames.push_back("draw_triangle_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_VERTEX:
                _internalFuncNames.push_back("draw_vertex");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_VERTEX_COLOR:
                _internalFuncNames.push_back("draw_vertex_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_VERTEX_TEXTURE:
                _internalFuncNames.push_back("draw_vertex_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DRAW_VERTEX_TEXTURE_COLOR:
                _internalFuncNames.push_back("draw_vertex_texture_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_ADD:
                _internalFuncNames.push_back("ds_grid_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_ADD_DISK:
                _internalFuncNames.push_back("ds_grid_add_disk");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_ADD_GRID_REGION:
                _internalFuncNames.push_back("ds_grid_add_grid_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_ADD_REGION:
                _internalFuncNames.push_back("ds_grid_add_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_CLEAR:
                _internalFuncNames.push_back("ds_grid_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_COPY:
                _internalFuncNames.push_back("ds_grid_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_CREATE:
                _internalFuncNames.push_back("ds_grid_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_DESTROY:
                _internalFuncNames.push_back("ds_grid_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET:
                _internalFuncNames.push_back("ds_grid_get");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_DISK_MAX:
                _internalFuncNames.push_back("ds_grid_get_disk_max");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_DISK_MEAN:
                _internalFuncNames.push_back("ds_grid_get_disk_mean");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_DISK_MIN:
                _internalFuncNames.push_back("ds_grid_get_disk_min");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_DISK_SUM:
                _internalFuncNames.push_back("ds_grid_get_disk_sum");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_MAX:
                _internalFuncNames.push_back("ds_grid_get_max");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_MEAN:
                _internalFuncNames.push_back("ds_grid_get_mean");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_MIN:
                _internalFuncNames.push_back("ds_grid_get_min");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_GET_SUM:
                _internalFuncNames.push_back("ds_grid_get_sum");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_HEIGHT:
                _internalFuncNames.push_back("ds_grid_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_MULTIPLY:
                _internalFuncNames.push_back("ds_grid_multiply");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_MULTIPLY_DISK:
                _internalFuncNames.push_back("ds_grid_multiply_disk");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_MULTIPLY_GRID_REGION:
                _internalFuncNames.push_back("ds_grid_multiply_grid_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_MULTIPLY_REGION:
                _internalFuncNames.push_back("ds_grid_multiply_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_READ:
                _internalFuncNames.push_back("ds_grid_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_RESIZE:
                _internalFuncNames.push_back("ds_grid_resize");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_SET:
                _internalFuncNames.push_back("ds_grid_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_SET_DISK:
                _internalFuncNames.push_back("ds_grid_set_disk");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_SET_GRID_REGION:
                _internalFuncNames.push_back("ds_grid_set_grid_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_SET_REGION:
                _internalFuncNames.push_back("ds_grid_set_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_SHUFFLE:
                _internalFuncNames.push_back("ds_grid_shuffle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_VALUE_DISK_EXISTS:
                _internalFuncNames.push_back("ds_grid_value_disk_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_VALUE_DISK_X:
                _internalFuncNames.push_back("ds_grid_value_disk_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_VALUE_DISK_Y:
                _internalFuncNames.push_back("ds_grid_value_disk_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_VALUE_EXISTS:
                _internalFuncNames.push_back("ds_grid_value_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_VALUE_X:
                _internalFuncNames.push_back("ds_grid_value_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_VALUE_Y:
                _internalFuncNames.push_back("ds_grid_value_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_WIDTH:
                _internalFuncNames.push_back("ds_grid_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_GRID_WRITE:
                _internalFuncNames.push_back("ds_grid_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_ADD:
                _internalFuncNames.push_back("ds_list_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_CLEAR:
                _internalFuncNames.push_back("ds_list_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_COPY:
                _internalFuncNames.push_back("ds_list_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_CREATE:
                _internalFuncNames.push_back("ds_list_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_DELETE:
                _internalFuncNames.push_back("ds_list_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_DESTROY:
                _internalFuncNames.push_back("ds_list_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_EMPTY:
                _internalFuncNames.push_back("ds_list_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_FIND_INDEX:
                _internalFuncNames.push_back("ds_list_find_index");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_FIND_VALUE:
                _internalFuncNames.push_back("ds_list_find_value");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_INSERT:
                _internalFuncNames.push_back("ds_list_insert");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_READ:
                _internalFuncNames.push_back("ds_list_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_REPLACE:
                _internalFuncNames.push_back("ds_list_replace");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_SHUFFLE:
                _internalFuncNames.push_back("ds_list_shuffle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_SIZE:
                _internalFuncNames.push_back("ds_list_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_SORT:
                _internalFuncNames.push_back("ds_list_sort");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_LIST_WRITE:
                _internalFuncNames.push_back("ds_list_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_ADD:
                _internalFuncNames.push_back("ds_map_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_CLEAR:
                _internalFuncNames.push_back("ds_map_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_COPY:
                _internalFuncNames.push_back("ds_map_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_CREATE:
                _internalFuncNames.push_back("ds_map_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_DELETE:
                _internalFuncNames.push_back("ds_map_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_DESTROY:
                _internalFuncNames.push_back("ds_map_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_EMPTY:
                _internalFuncNames.push_back("ds_map_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_EXISTS:
                _internalFuncNames.push_back("ds_map_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_FIND_FIRST:
                _internalFuncNames.push_back("ds_map_find_first");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_FIND_LAST:
                _internalFuncNames.push_back("ds_map_find_last");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_FIND_NEXT:
                _internalFuncNames.push_back("ds_map_find_next");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_FIND_PREVIOUS:
                _internalFuncNames.push_back("ds_map_find_previous");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_FIND_VALUE:
                _internalFuncNames.push_back("ds_map_find_value");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_READ:
                _internalFuncNames.push_back("ds_map_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_REPLACE:
                _internalFuncNames.push_back("ds_map_replace");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_SIZE:
                _internalFuncNames.push_back("ds_map_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_MAP_WRITE:
                _internalFuncNames.push_back("ds_map_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_ADD:
                _internalFuncNames.push_back("ds_priority_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_CHANGE_PRIORITY:
                _internalFuncNames.push_back("ds_priority_change_priority");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_CLEAR:
                _internalFuncNames.push_back("ds_priority_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_COPY:
                _internalFuncNames.push_back("ds_priority_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_CREATE:
                _internalFuncNames.push_back("ds_priority_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_DELETE_MAX:
                _internalFuncNames.push_back("ds_priority_delete_max");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_DELETE_MIN:
                _internalFuncNames.push_back("ds_priority_delete_min");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_DELETE_VALUE:
                _internalFuncNames.push_back("ds_priority_delete_value");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_DESTROY:
                _internalFuncNames.push_back("ds_priority_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_EMPTY:
                _internalFuncNames.push_back("ds_priority_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_FIND_MAX:
                _internalFuncNames.push_back("ds_priority_find_max");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_FIND_MIN:
                _internalFuncNames.push_back("ds_priority_find_min");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_FIND_PRIORITY:
                _internalFuncNames.push_back("ds_priority_find_priority");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_READ:
                _internalFuncNames.push_back("ds_priority_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_SIZE:
                _internalFuncNames.push_back("ds_priority_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_PRIORITY_WRITE:
                _internalFuncNames.push_back("ds_priority_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_CLEAR:
                _internalFuncNames.push_back("ds_queue_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_COPY:
                _internalFuncNames.push_back("ds_queue_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_CREATE:
                _internalFuncNames.push_back("ds_queue_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_DEQUEUE:
                _internalFuncNames.push_back("ds_queue_dequeue");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_DESTROY:
                _internalFuncNames.push_back("ds_queue_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_EMPTY:
                _internalFuncNames.push_back("ds_queue_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_ENQUEUE:
                _internalFuncNames.push_back("ds_queue_enqueue");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_HEAD:
                _internalFuncNames.push_back("ds_queue_head");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_READ:
                _internalFuncNames.push_back("ds_queue_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_SIZE:
                _internalFuncNames.push_back("ds_queue_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_TAIL:
                _internalFuncNames.push_back("ds_queue_tail");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_QUEUE_WRITE:
                _internalFuncNames.push_back("ds_queue_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_SET_PRECISION:
                _internalFuncNames.push_back("ds_set_precision");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_CLEAR:
                _internalFuncNames.push_back("ds_stack_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_COPY:
                _internalFuncNames.push_back("ds_stack_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_CREATE:
                _internalFuncNames.push_back("ds_stack_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_DESTROY:
                _internalFuncNames.push_back("ds_stack_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_EMPTY:
                _internalFuncNames.push_back("ds_stack_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_POP:
                _internalFuncNames.push_back("ds_stack_pop");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_PUSH:
                _internalFuncNames.push_back("ds_stack_push");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_READ:
                _internalFuncNames.push_back("ds_stack_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_SIZE:
                _internalFuncNames.push_back("ds_stack_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_TOP:
                _internalFuncNames.push_back("ds_stack_top");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case DS_STACK_WRITE:
                _internalFuncNames.push_back("ds_stack_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EFFECT_CLEAR:
                _internalFuncNames.push_back("effect_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EFFECT_CREATE_ABOVE:
                _internalFuncNames.push_back("effect_create_above");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EFFECT_CREATE_BELOW:
                _internalFuncNames.push_back("effect_create_below");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ENVIRONMENT_GET_VARIABLE:
                _internalFuncNames.push_back("environment_get_variable");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EVENT_INHERITED:
                _internalFuncNames.push_back("event_inherited");
                _gmlFuncs.push_back(&Runtime::event_inherited);
                break;
            case EVENT_PERFORM:
                _internalFuncNames.push_back("event_perform");
                _gmlFuncs.push_back(&Runtime::event_perform);
                break;
            case EVENT_PERFORM_OBJECT:
                _internalFuncNames.push_back("event_perform_object");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EVENT_USER:
                _internalFuncNames.push_back("event_user");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXECUTE_FILE:
                _internalFuncNames.push_back("execute_file");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXECUTE_PROGRAM:
                _internalFuncNames.push_back("execute_program");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXECUTE_SHELL:
                _internalFuncNames.push_back("execute_shell");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXECUTE_STRING:
                _internalFuncNames.push_back("execute_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXP:
                _internalFuncNames.push_back("exp");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXPORT_INCLUDE_FILE:
                _internalFuncNames.push_back("export_include_file");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXPORT_INCLUDE_FILE_LOCATION:
                _internalFuncNames.push_back("export_include_file_location");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXTERNAL_CALL:
                _internalFuncNames.push_back("external_call");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXTERNAL_DEFINE:
                _internalFuncNames.push_back("external_define");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case EXTERNAL_FREE:
                _internalFuncNames.push_back("external_free");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_ATTRIBUTES:
                _internalFuncNames.push_back("file_attributes");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_BIN_CLOSE:
                _internalFuncNames.push_back("file_bin_close");
                _gmlFuncs.push_back(&Runtime::file_bin_close);
                break;
            case FILE_BIN_OPEN:
                _internalFuncNames.push_back("file_bin_open");
                _gmlFuncs.push_back(&Runtime::file_bin_open);
                break;
            case FILE_BIN_POSITION:
                _internalFuncNames.push_back("file_bin_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_BIN_READ_BYTE:
                _internalFuncNames.push_back("file_bin_read_byte");
                _gmlFuncs.push_back(&Runtime::file_bin_read_byte);
                break;
            case FILE_BIN_REWRITE:
                _internalFuncNames.push_back("file_bin_rewrite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_BIN_SEEK:
                _internalFuncNames.push_back("file_bin_seek");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_BIN_SIZE:
                _internalFuncNames.push_back("file_bin_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_BIN_WRITE_BYTE:
                _internalFuncNames.push_back("file_bin_write_byte");
                _gmlFuncs.push_back(&Runtime::file_bin_write_byte);
                break;
            case FILE_COPY:
                _internalFuncNames.push_back("file_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_DELETE:
                _internalFuncNames.push_back("file_delete");
                _gmlFuncs.push_back(&Runtime::file_delete);
                break;
            case FILE_EXISTS:
                _internalFuncNames.push_back("file_exists");
                _gmlFuncs.push_back(&Runtime::file_exists);
                break;
            case FILE_FIND_CLOSE:
                _internalFuncNames.push_back("file_find_close");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_FIND_FIRST:
                _internalFuncNames.push_back("file_find_first");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_FIND_NEXT:
                _internalFuncNames.push_back("file_find_next");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_RENAME:
                _internalFuncNames.push_back("file_rename");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_CLOSE:
                _internalFuncNames.push_back("file_text_close");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_EOF:
                _internalFuncNames.push_back("file_text_eof");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_EOLN:
                _internalFuncNames.push_back("file_text_eoln");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_OPEN_APPEND:
                _internalFuncNames.push_back("file_text_open_append");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_OPEN_READ:
                _internalFuncNames.push_back("file_text_open_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_OPEN_WRITE:
                _internalFuncNames.push_back("file_text_open_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_READ_REAL:
                _internalFuncNames.push_back("file_text_read_real");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_READ_STRING:
                _internalFuncNames.push_back("file_text_read_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_READLN:
                _internalFuncNames.push_back("file_text_readln");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_WRITE_REAL:
                _internalFuncNames.push_back("file_text_write_real");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_WRITE_STRING:
                _internalFuncNames.push_back("file_text_write_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILE_TEXT_WRITELN:
                _internalFuncNames.push_back("file_text_writeln");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILENAME_CHANGE_EXT:
                _internalFuncNames.push_back("filename_change_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILENAME_DIR:
                _internalFuncNames.push_back("filename_dir");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILENAME_DRIVE:
                _internalFuncNames.push_back("filename_drive");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILENAME_EXT:
                _internalFuncNames.push_back("filename_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILENAME_NAME:
                _internalFuncNames.push_back("filename_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FILENAME_PATH:
                _internalFuncNames.push_back("filename_path");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FLOOR:
                _internalFuncNames.push_back("floor");
                _gmlFuncs.push_back(&Runtime::floor);
                break;
            case FONT_ADD:
                _internalFuncNames.push_back("font_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_ADD_SPRITE:
                _internalFuncNames.push_back("font_add_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_DELETE:
                _internalFuncNames.push_back("font_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_EXISTS:
                _internalFuncNames.push_back("font_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_GET_BOLD:
                _internalFuncNames.push_back("font_get_bold");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_GET_FIRST:
                _internalFuncNames.push_back("font_get_first");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_GET_FONTNAME:
                _internalFuncNames.push_back("font_get_fontname");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_GET_ITALIC:
                _internalFuncNames.push_back("font_get_italic");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_GET_LAST:
                _internalFuncNames.push_back("font_get_last");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_GET_NAME:
                _internalFuncNames.push_back("font_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_REPLACE:
                _internalFuncNames.push_back("font_replace");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FONT_REPLACE_SPRITE:
                _internalFuncNames.push_back("font_replace_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case FRAC:
                _internalFuncNames.push_back("frac");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GAME_END:
                _internalFuncNames.push_back("game_end");
                _gmlFuncs.push_back(&Runtime::game_end);
                break;
            case GAME_LOAD:
                _internalFuncNames.push_back("game_load");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GAME_RESTART:
                _internalFuncNames.push_back("game_restart");
                _gmlFuncs.push_back(&Runtime::game_restart);
                break;
            case GAME_SAVE:
                _internalFuncNames.push_back("game_save");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_COLOR:
                _internalFuncNames.push_back("get_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_DIRECTORY:
                _internalFuncNames.push_back("get_directory");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_DIRECTORY_ALT:
                _internalFuncNames.push_back("get_directory_alt");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_FUNCTION_ADDRESS:
                _internalFuncNames.push_back("get_function_address");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_INTEGER:
                _internalFuncNames.push_back("get_integer");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_OPEN_FILENAME:
                _internalFuncNames.push_back("get_open_filename");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_SAVE_FILENAME:
                _internalFuncNames.push_back("get_save_filename");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case GET_STRING:
                _internalFuncNames.push_back("get_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_ADD:
                _internalFuncNames.push_back("highscore_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_ADD_CURRENT:
                _internalFuncNames.push_back("highscore_add_current");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_CLEAR:
                _internalFuncNames.push_back("highscore_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_NAME:
                _internalFuncNames.push_back("highscore_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_SET_COLOR:
                _internalFuncNames.push_back("highscore_set_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_SET_STRINGS:
                _internalFuncNames.push_back("highscore_set_strings");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_SHOW:
                _internalFuncNames.push_back("highscore_show");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_SHOW_EXT:
                _internalFuncNames.push_back("highscore_show_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case HIGHSCORE_VALUE:
                _internalFuncNames.push_back("highscore_value");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_CLOSE:
                _internalFuncNames.push_back("ini_close");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_KEY_DELETE:
                _internalFuncNames.push_back("ini_key_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_KEY_EXISTS:
                _internalFuncNames.push_back("ini_key_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_OPEN:
                _internalFuncNames.push_back("ini_open");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_READ_REAL:
                _internalFuncNames.push_back("ini_read_real");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_READ_STRING:
                _internalFuncNames.push_back("ini_read_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_SECTION_DELETE:
                _internalFuncNames.push_back("ini_section_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_SECTION_EXISTS:
                _internalFuncNames.push_back("ini_section_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_WRITE_REAL:
                _internalFuncNames.push_back("ini_write_real");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INI_WRITE_STRING:
                _internalFuncNames.push_back("ini_write_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_ACTIVATE_ALL:
                _internalFuncNames.push_back("instance_activate_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_ACTIVATE_OBJECT:
                _internalFuncNames.push_back("instance_activate_object");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_ACTIVATE_REGION:
                _internalFuncNames.push_back("instance_activate_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_CHANGE:
                _internalFuncNames.push_back("instance_change");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_COPY:
                _internalFuncNames.push_back("instance_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_CREATE:
                _internalFuncNames.push_back("instance_create");
                _gmlFuncs.push_back(&Runtime::instance_create);
                break;
            case INSTANCE_DEACTIVATE_ALL:
                _internalFuncNames.push_back("instance_deactivate_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_DEACTIVATE_OBJECT:
                _internalFuncNames.push_back("instance_deactivate_object");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_DEACTIVATE_REGION:
                _internalFuncNames.push_back("instance_deactivate_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_DESTROY:
                _internalFuncNames.push_back("instance_destroy");
                _gmlFuncs.push_back(&Runtime::instance_destroy);
                break;
            case INSTANCE_EXISTS:
                _internalFuncNames.push_back("instance_exists");
                _gmlFuncs.push_back(&Runtime::instance_exists);
                break;
            case INSTANCE_FIND:
                _internalFuncNames.push_back("instance_find");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_FURTHEST:
                _internalFuncNames.push_back("instance_furthest");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_NEAREST:
                _internalFuncNames.push_back("instance_nearest");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_NUMBER:
                _internalFuncNames.push_back("instance_number");
                _gmlFuncs.push_back(&Runtime::instance_number);
                break;
            case INSTANCE_PLACE:
                _internalFuncNames.push_back("instance_place");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case INSTANCE_POSITION:
                _internalFuncNames.push_back("instance_position");
                _gmlFuncs.push_back(&Runtime::instance_position);
                break;
            case IO_CLEAR:
                _internalFuncNames.push_back("io_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case IO_HANDLE:
                _internalFuncNames.push_back("io_handle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case IRANDOM:
                _internalFuncNames.push_back("irandom");
                _gmlFuncs.push_back(&Runtime::irandom);
                break;
            case IRANDOM_RANGE:
                _internalFuncNames.push_back("irandom_range");
                _gmlFuncs.push_back(&Runtime::irandom_range);
                break;
            case IS_REAL:
                _internalFuncNames.push_back("is_real");
                _gmlFuncs.push_back(&Runtime::is_real);
                break;
            case IS_STRING:
                _internalFuncNames.push_back("is_string");
                _gmlFuncs.push_back(&Runtime::is_string);
                break;
            case JOYSTICK_AXES:
                _internalFuncNames.push_back("joystick_axes");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_BUTTONS:
                _internalFuncNames.push_back("joystick_buttons");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_CHECK_BUTTON:
                _internalFuncNames.push_back("joystick_check_button");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_DIRECTION:
                _internalFuncNames.push_back("joystick_direction");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_EXISTS:
                _internalFuncNames.push_back("joystick_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_HAS_POV:
                _internalFuncNames.push_back("joystick_has_pov");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_NAME:
                _internalFuncNames.push_back("joystick_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_POV:
                _internalFuncNames.push_back("joystick_pov");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_RPOS:
                _internalFuncNames.push_back("joystick_rpos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_UPOS:
                _internalFuncNames.push_back("joystick_upos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_VPOS:
                _internalFuncNames.push_back("joystick_vpos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_XPOS:
                _internalFuncNames.push_back("joystick_xpos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_YPOS:
                _internalFuncNames.push_back("joystick_ypos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case JOYSTICK_ZPOS:
                _internalFuncNames.push_back("joystick_zpos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_CHECK:
                _internalFuncNames.push_back("keyboard_check");
                _gmlFuncs.push_back(&Runtime::keyboard_check);
                break;
            case KEYBOARD_CHECK_DIRECT:
                _internalFuncNames.push_back("keyboard_check_direct");
                _gmlFuncs.push_back(&Runtime::keyboard_check_direct);
                break;
            case KEYBOARD_CHECK_PRESSED:
                _internalFuncNames.push_back("keyboard_check_pressed");
                _gmlFuncs.push_back(&Runtime::keyboard_check_pressed);
                break;
            case KEYBOARD_CHECK_RELEASED:
                _internalFuncNames.push_back("keyboard_check_released");
                _gmlFuncs.push_back(&Runtime::keyboard_check_released);
                break;
            case KEYBOARD_CLEAR:
                _internalFuncNames.push_back("keyboard_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_GET_MAP:
                _internalFuncNames.push_back("keyboard_get_map");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_GET_NUMLOCK:
                _internalFuncNames.push_back("keyboard_get_numlock");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_KEY_PRESS:
                _internalFuncNames.push_back("keyboard_key_press");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_KEY_RELEASE:
                _internalFuncNames.push_back("keyboard_key_release");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_SET_MAP:
                _internalFuncNames.push_back("keyboard_set_map");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_SET_NUMLOCK:
                _internalFuncNames.push_back("keyboard_set_numlock");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_UNSET_MAP:
                _internalFuncNames.push_back("keyboard_unset_map");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case KEYBOARD_WAIT:
                _internalFuncNames.push_back("keyboard_wait");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case LENGTHDIR_X:
                _internalFuncNames.push_back("lengthdir_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case LENGTHDIR_Y:
                _internalFuncNames.push_back("lengthdir_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case LN:
                _internalFuncNames.push_back("ln");
                _gmlFuncs.push_back(&Runtime::ln);
                break;
            case LOAD_INFO:
                _internalFuncNames.push_back("load_info");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case LOG10:
                _internalFuncNames.push_back("log10");
                _gmlFuncs.push_back(&Runtime::log10);
                break;
            case LOG2:
                _internalFuncNames.push_back("log2");
                _gmlFuncs.push_back(&Runtime::log2);
                break;
            case LOGN:
                _internalFuncNames.push_back("logn");
                _gmlFuncs.push_back(&Runtime::logn);
                break;
            case MAKE_COLOR_HSV:
                _internalFuncNames.push_back("make_color_hsv");
                _gmlFuncs.push_back(&Runtime::make_color_hsv);
                break;
            case MAKE_COLOR_RGB:
                _internalFuncNames.push_back("make_color_rgb");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MAX:
                _internalFuncNames.push_back("max");
                _gmlFuncs.push_back(&Runtime::max);
                break;
            case MCI_COMMAND:
                _internalFuncNames.push_back("mci_command");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MEAN:
                _internalFuncNames.push_back("mean");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MEDIAN:
                _internalFuncNames.push_back("median");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MERGE_COLOR:
                _internalFuncNames.push_back("merge_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_ALPHA:
                _internalFuncNames.push_back("message_alpha");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_BACKGROUND:
                _internalFuncNames.push_back("message_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_BUTTON:
                _internalFuncNames.push_back("message_button");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_BUTTON_FONT:
                _internalFuncNames.push_back("message_button_font");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_CAPTION:
                _internalFuncNames.push_back("message_caption");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_INPUT_COLOR:
                _internalFuncNames.push_back("message_input_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_INPUT_FONT:
                _internalFuncNames.push_back("message_input_font");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_MOUSE_COLOR:
                _internalFuncNames.push_back("message_mouse_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_POSITION:
                _internalFuncNames.push_back("message_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_SIZE:
                _internalFuncNames.push_back("message_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_TEXT_CHARSET:
                _internalFuncNames.push_back("message_text_charset");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MESSAGE_TEXT_FONT:
                _internalFuncNames.push_back("message_text_font");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MIN:
                _internalFuncNames.push_back("min");
                _gmlFuncs.push_back(&Runtime::min);
                break;
            case MOTION_ADD:
                _internalFuncNames.push_back("motion_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOTION_SET:
                _internalFuncNames.push_back("motion_set");
                _gmlFuncs.push_back(&Runtime::motion_set);
                break;
            case MOUSE_CHECK_BUTTON:
                _internalFuncNames.push_back("mouse_check_button");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOUSE_CHECK_BUTTON_PRESSED:
                _internalFuncNames.push_back("mouse_check_button_pressed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOUSE_CHECK_BUTTON_RELEASED:
                _internalFuncNames.push_back("mouse_check_button_released");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOUSE_CLEAR:
                _internalFuncNames.push_back("mouse_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOUSE_WAIT:
                _internalFuncNames.push_back("mouse_wait");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOUSE_WHEEL_DOWN:
                _internalFuncNames.push_back("mouse_wheel_down");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOUSE_WHEEL_UP:
                _internalFuncNames.push_back("mouse_wheel_up");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_BOUNCE_ALL:
                _internalFuncNames.push_back("move_bounce_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_BOUNCE_SOLID:
                _internalFuncNames.push_back("move_bounce_solid");
                _gmlFuncs.push_back(&Runtime::move_bounce_solid);
                break;
            case MOVE_CONTACT_ALL:
                _internalFuncNames.push_back("move_contact_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_CONTACT_SOLID:
                _internalFuncNames.push_back("move_contact_solid");
                _gmlFuncs.push_back(&Runtime::move_contact_solid);
                break;
            case MOVE_OUTSIDE_ALL:
                _internalFuncNames.push_back("move_outside_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_OUTSIDE_SOLID:
                _internalFuncNames.push_back("move_outside_solid");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_RANDOM:
                _internalFuncNames.push_back("move_random");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_SNAP:
                _internalFuncNames.push_back("move_snap");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_TOWARDS_POINT:
                _internalFuncNames.push_back("move_towards_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MOVE_WRAP:
                _internalFuncNames.push_back("move_wrap");
                _gmlFuncs.push_back(&Runtime::move_wrap);
                break;
            case MP_GRID_ADD_CELL:
                _internalFuncNames.push_back("mp_grid_add_cell");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_ADD_INSTANCES:
                _internalFuncNames.push_back("mp_grid_add_instances");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_ADD_RECTANGLE:
                _internalFuncNames.push_back("mp_grid_add_rectangle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_CLEAR_ALL:
                _internalFuncNames.push_back("mp_grid_clear_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_CLEAR_CELL:
                _internalFuncNames.push_back("mp_grid_clear_cell");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_CLEAR_RECTANGLE:
                _internalFuncNames.push_back("mp_grid_clear_rectangle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_CREATE:
                _internalFuncNames.push_back("mp_grid_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_DESTROY:
                _internalFuncNames.push_back("mp_grid_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_DRAW:
                _internalFuncNames.push_back("mp_grid_draw");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_GRID_PATH:
                _internalFuncNames.push_back("mp_grid_path");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_LINEAR_PATH:
                _internalFuncNames.push_back("mp_linear_path");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_LINEAR_PATH_OBJECT:
                _internalFuncNames.push_back("mp_linear_path_object");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_LINEAR_STEP:
                _internalFuncNames.push_back("mp_linear_step");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_LINEAR_STEP_OBJECT:
                _internalFuncNames.push_back("mp_linear_step_object");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_POTENTIAL_PATH:
                _internalFuncNames.push_back("mp_potential_path");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_POTENTIAL_PATH_OBJECT:
                _internalFuncNames.push_back("mp_potential_path_object");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_POTENTIAL_SETTINGS:
                _internalFuncNames.push_back("mp_potential_settings");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_POTENTIAL_STEP:
                _internalFuncNames.push_back("mp_potential_step");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MP_POTENTIAL_STEP_OBJECT:
                _internalFuncNames.push_back("mp_potential_step_object");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_CONNECT_STATUS:
                _internalFuncNames.push_back("mplay_connect_status");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_DATA_MODE:
                _internalFuncNames.push_back("mplay_data_mode");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_DATA_READ:
                _internalFuncNames.push_back("mplay_data_read");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_DATA_WRITE:
                _internalFuncNames.push_back("mplay_data_write");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_END:
                _internalFuncNames.push_back("mplay_end");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_INIT_IPX:
                _internalFuncNames.push_back("mplay_init_ipx");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_INIT_MODEM:
                _internalFuncNames.push_back("mplay_init_modem");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_INIT_SERIAL:
                _internalFuncNames.push_back("mplay_init_serial");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_INIT_TCPIP:
                _internalFuncNames.push_back("mplay_init_tcpip");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_IPADDRESS:
                _internalFuncNames.push_back("mplay_ipaddress");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_CLEAR:
                _internalFuncNames.push_back("mplay_message_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_COUNT:
                _internalFuncNames.push_back("mplay_message_count");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_ID:
                _internalFuncNames.push_back("mplay_message_id");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_NAME:
                _internalFuncNames.push_back("mplay_message_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_PLAYER:
                _internalFuncNames.push_back("mplay_message_player");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_RECEIVE:
                _internalFuncNames.push_back("mplay_message_receive");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_SEND:
                _internalFuncNames.push_back("mplay_message_send");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_SEND_GUARANTEED:
                _internalFuncNames.push_back("mplay_message_send_guaranteed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_MESSAGE_VALUE:
                _internalFuncNames.push_back("mplay_message_value");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_PLAYER_FIND:
                _internalFuncNames.push_back("mplay_player_find");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_PLAYER_ID:
                _internalFuncNames.push_back("mplay_player_id");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_PLAYER_NAME:
                _internalFuncNames.push_back("mplay_player_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_SESSION_CREATE:
                _internalFuncNames.push_back("mplay_session_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_SESSION_END:
                _internalFuncNames.push_back("mplay_session_end");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_SESSION_FIND:
                _internalFuncNames.push_back("mplay_session_find");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_SESSION_JOIN:
                _internalFuncNames.push_back("mplay_session_join");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_SESSION_MODE:
                _internalFuncNames.push_back("mplay_session_mode");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_SESSION_NAME:
                _internalFuncNames.push_back("mplay_session_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case MPLAY_SESSION_STATUS:
                _internalFuncNames.push_back("mplay_session_status");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_ADD:
                _internalFuncNames.push_back("object_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_DELETE:
                _internalFuncNames.push_back("object_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_EVENT_ADD:
                _internalFuncNames.push_back("object_event_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_EVENT_CLEAR:
                _internalFuncNames.push_back("object_event_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_EXISTS:
                _internalFuncNames.push_back("object_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_DEPTH:
                _internalFuncNames.push_back("object_get_depth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_MASK:
                _internalFuncNames.push_back("object_get_mask");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_NAME:
                _internalFuncNames.push_back("object_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_PARENT:
                _internalFuncNames.push_back("object_get_parent");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_PERSISTENT:
                _internalFuncNames.push_back("object_get_persistent");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_SOLID:
                _internalFuncNames.push_back("object_get_solid");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_SPRITE:
                _internalFuncNames.push_back("object_get_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_GET_VISIBLE:
                _internalFuncNames.push_back("object_get_visible");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_IS_ANCESTOR:
                _internalFuncNames.push_back("object_is_ancestor");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_SET_DEPTH:
                _internalFuncNames.push_back("object_set_depth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_SET_MASK:
                _internalFuncNames.push_back("object_set_mask");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_SET_PARENT:
                _internalFuncNames.push_back("object_set_parent");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_SET_PERSISTENT:
                _internalFuncNames.push_back("object_set_persistent");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_SET_SOLID:
                _internalFuncNames.push_back("object_set_solid");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_SET_SPRITE:
                _internalFuncNames.push_back("object_set_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case OBJECT_SET_VISIBLE:
                _internalFuncNames.push_back("object_set_visible");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ORD:
                _internalFuncNames.push_back("ord");
                _gmlFuncs.push_back(&Runtime::ord);
                break;
            case PARAMETER_COUNT:
                _internalFuncNames.push_back("parameter_count");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PARAMETER_STRING:
                _internalFuncNames.push_back("parameter_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_ATTRACTOR_CLEAR:
                _internalFuncNames.push_back("part_attractor_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_ATTRACTOR_CREATE:
                _internalFuncNames.push_back("part_attractor_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_ATTRACTOR_DESTROY:
                _internalFuncNames.push_back("part_attractor_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_ATTRACTOR_DESTROY_ALL:
                _internalFuncNames.push_back("part_attractor_destroy_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_ATTRACTOR_EXISTS:
                _internalFuncNames.push_back("part_attractor_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_ATTRACTOR_FORCE:
                _internalFuncNames.push_back("part_attractor_force");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_ATTRACTOR_POSITION:
                _internalFuncNames.push_back("part_attractor_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_CLEAR:
                _internalFuncNames.push_back("part_changer_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_CREATE:
                _internalFuncNames.push_back("part_changer_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_DESTROY:
                _internalFuncNames.push_back("part_changer_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_DESTROY_ALL:
                _internalFuncNames.push_back("part_changer_destroy_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_EXISTS:
                _internalFuncNames.push_back("part_changer_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_KIND:
                _internalFuncNames.push_back("part_changer_kind");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_REGION:
                _internalFuncNames.push_back("part_changer_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_CHANGER_TYPES:
                _internalFuncNames.push_back("part_changer_types");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_CLEAR:
                _internalFuncNames.push_back("part_deflector_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_CREATE:
                _internalFuncNames.push_back("part_deflector_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_DESTROY:
                _internalFuncNames.push_back("part_deflector_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_DESTROY_ALL:
                _internalFuncNames.push_back("part_deflector_destroy_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_EXISTS:
                _internalFuncNames.push_back("part_deflector_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_FRICTION:
                _internalFuncNames.push_back("part_deflector_friction");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_KIND:
                _internalFuncNames.push_back("part_deflector_kind");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DEFLECTOR_REGION:
                _internalFuncNames.push_back("part_deflector_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DESTROYER_CLEAR:
                _internalFuncNames.push_back("part_destroyer_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DESTROYER_CREATE:
                _internalFuncNames.push_back("part_destroyer_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DESTROYER_DESTROY:
                _internalFuncNames.push_back("part_destroyer_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DESTROYER_DESTROY_ALL:
                _internalFuncNames.push_back("part_destroyer_destroy_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DESTROYER_EXISTS:
                _internalFuncNames.push_back("part_destroyer_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_DESTROYER_REGION:
                _internalFuncNames.push_back("part_destroyer_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_BURST:
                _internalFuncNames.push_back("part_emitter_burst");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_CLEAR:
                _internalFuncNames.push_back("part_emitter_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_CREATE:
                _internalFuncNames.push_back("part_emitter_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_DESTROY:
                _internalFuncNames.push_back("part_emitter_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_DESTROY_ALL:
                _internalFuncNames.push_back("part_emitter_destroy_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_EXISTS:
                _internalFuncNames.push_back("part_emitter_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_REGION:
                _internalFuncNames.push_back("part_emitter_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_EMITTER_STREAM:
                _internalFuncNames.push_back("part_emitter_stream");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_PARTICLES_CLEAR:
                _internalFuncNames.push_back("part_particles_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_PARTICLES_COUNT:
                _internalFuncNames.push_back("part_particles_count");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_PARTICLES_CREATE:
                _internalFuncNames.push_back("part_particles_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_PARTICLES_CREATE_COLOR:
                _internalFuncNames.push_back("part_particles_create_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_AUTOMATIC_DRAW:
                _internalFuncNames.push_back("part_system_automatic_draw");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_AUTOMATIC_UPDATE:
                _internalFuncNames.push_back("part_system_automatic_update");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_CLEAR:
                _internalFuncNames.push_back("part_system_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_CREATE:
                _internalFuncNames.push_back("part_system_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_DEPTH:
                _internalFuncNames.push_back("part_system_depth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_DESTROY:
                _internalFuncNames.push_back("part_system_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_DRAW_ORDER:
                _internalFuncNames.push_back("part_system_draw_order");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_DRAWIT:
                _internalFuncNames.push_back("part_system_drawit");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_EXISTS:
                _internalFuncNames.push_back("part_system_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_POSITION:
                _internalFuncNames.push_back("part_system_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_SYSTEM_UPDATE:
                _internalFuncNames.push_back("part_system_update");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_ALPHA1:
                _internalFuncNames.push_back("part_type_alpha1");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_ALPHA2:
                _internalFuncNames.push_back("part_type_alpha2");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_ALPHA3:
                _internalFuncNames.push_back("part_type_alpha3");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_BLEND:
                _internalFuncNames.push_back("part_type_blend");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_CLEAR:
                _internalFuncNames.push_back("part_type_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_COLOR_HSV:
                _internalFuncNames.push_back("part_type_color_hsv");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_COLOR_MIX:
                _internalFuncNames.push_back("part_type_color_mix");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_COLOR_RGB:
                _internalFuncNames.push_back("part_type_color_rgb");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_COLOR1:
                _internalFuncNames.push_back("part_type_color1");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_COLOR2:
                _internalFuncNames.push_back("part_type_color2");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_COLOR3:
                _internalFuncNames.push_back("part_type_color3");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_CREATE:
                _internalFuncNames.push_back("part_type_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_DEATH:
                _internalFuncNames.push_back("part_type_death");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_DESTROY:
                _internalFuncNames.push_back("part_type_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_DIRECTION:
                _internalFuncNames.push_back("part_type_direction");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_EXISTS:
                _internalFuncNames.push_back("part_type_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_GRAVITY:
                _internalFuncNames.push_back("part_type_gravity");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_LIFE:
                _internalFuncNames.push_back("part_type_life");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_ORIENTATION:
                _internalFuncNames.push_back("part_type_orientation");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_SCALE:
                _internalFuncNames.push_back("part_type_scale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_SHAPE:
                _internalFuncNames.push_back("part_type_shape");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_SIZE:
                _internalFuncNames.push_back("part_type_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_SPEED:
                _internalFuncNames.push_back("part_type_speed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_SPRITE:
                _internalFuncNames.push_back("part_type_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PART_TYPE_STEP:
                _internalFuncNames.push_back("part_type_step");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_ADD:
                _internalFuncNames.push_back("path_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_ADD_POINT:
                _internalFuncNames.push_back("path_add_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_APPEND:
                _internalFuncNames.push_back("path_append");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_ASSIGN:
                _internalFuncNames.push_back("path_assign");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_CHANGE_POINT:
                _internalFuncNames.push_back("path_change_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_CLEAR_POINTS:
                _internalFuncNames.push_back("path_clear_points");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_DELETE:
                _internalFuncNames.push_back("path_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_DELETE_POINT:
                _internalFuncNames.push_back("path_delete_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_DUPLICATE:
                _internalFuncNames.push_back("path_duplicate");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_END:
                _internalFuncNames.push_back("path_end");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_EXISTS:
                _internalFuncNames.push_back("path_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_FLIP:
                _internalFuncNames.push_back("path_flip");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_CLOSED:
                _internalFuncNames.push_back("path_get_closed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_KIND:
                _internalFuncNames.push_back("path_get_kind");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_LENGTH:
                _internalFuncNames.push_back("path_get_length");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_NAME:
                _internalFuncNames.push_back("path_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_NUMBER:
                _internalFuncNames.push_back("path_get_number");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_POINT_SPEED:
                _internalFuncNames.push_back("path_get_point_speed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_POINT_X:
                _internalFuncNames.push_back("path_get_point_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_POINT_Y:
                _internalFuncNames.push_back("path_get_point_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_PRECISION:
                _internalFuncNames.push_back("path_get_precision");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_SPEED:
                _internalFuncNames.push_back("path_get_speed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_X:
                _internalFuncNames.push_back("path_get_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_GET_Y:
                _internalFuncNames.push_back("path_get_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_INSERT_POINT:
                _internalFuncNames.push_back("path_insert_point");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_MIRROR:
                _internalFuncNames.push_back("path_mirror");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_REVERSE:
                _internalFuncNames.push_back("path_reverse");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_ROTATE:
                _internalFuncNames.push_back("path_rotate");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_SCALE:
                _internalFuncNames.push_back("path_scale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_SET_CLOSED:
                _internalFuncNames.push_back("path_set_closed");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_SET_KIND:
                _internalFuncNames.push_back("path_set_kind");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_SET_PRECISION:
                _internalFuncNames.push_back("path_set_precision");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_SHIFT:
                _internalFuncNames.push_back("path_shift");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PATH_START:
                _internalFuncNames.push_back("path_start");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PLACE_EMPTY:
                _internalFuncNames.push_back("place_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case PLACE_FREE:
                _internalFuncNames.push_back("place_free");
                _gmlFuncs.push_back(&Runtime::place_free);
                break;
            case PLACE_MEETING:
                _internalFuncNames.push_back("place_meeting");
                _gmlFuncs.push_back(&Runtime::place_meeting);
                break;
            case PLACE_SNAPPED:
                _internalFuncNames.push_back("place_snapped");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case POINT_DIRECTION:
                _internalFuncNames.push_back("point_direction");
                _gmlFuncs.push_back(&Runtime::point_direction);
                break;
            case POINT_DISTANCE:
                _internalFuncNames.push_back("point_distance");
                _gmlFuncs.push_back(&Runtime::point_distance);
                break;
            case POINT_DISTANCE_3D:
                _internalFuncNames.push_back("point_distance_3d");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case POSITION_CHANGE:
                _internalFuncNames.push_back("position_change");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case POSITION_DESTROY:
                _internalFuncNames.push_back("position_destroy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case POSITION_EMPTY:
                _internalFuncNames.push_back("position_empty");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case POSITION_MEETING:
                _internalFuncNames.push_back("position_meeting");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case POWER:
                _internalFuncNames.push_back("power");
                _gmlFuncs.push_back(&Runtime::power);
                break;
            case RADTODEG:
                _internalFuncNames.push_back("radtodeg");
                _gmlFuncs.push_back(&Runtime::radtodeg);
                break;
            case RANDOM:
                _internalFuncNames.push_back("random");
                _gmlFuncs.push_back(&Runtime::random);
                break;
            case RANDOM_GET_SEED:
                _internalFuncNames.push_back("random_get_seed");
                _gmlFuncs.push_back(&Runtime::random_get_seed);
                break;
            case RANDOM_RANGE:
                _internalFuncNames.push_back("random_range");
                _gmlFuncs.push_back(&Runtime::random_range);
                break;
            case RANDOM_SET_SEED:
                _internalFuncNames.push_back("random_set_seed");
                _gmlFuncs.push_back(&Runtime::random_set_seed);
                break;
            case RANDOMIZE:
                _internalFuncNames.push_back("randomize");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REAL:
                _internalFuncNames.push_back("real");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_EXISTS:
                _internalFuncNames.push_back("registry_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_EXISTS_EXT:
                _internalFuncNames.push_back("registry_exists_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_READ_REAL:
                _internalFuncNames.push_back("registry_read_real");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_READ_REAL_EXT:
                _internalFuncNames.push_back("registry_read_real_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_READ_STRING:
                _internalFuncNames.push_back("registry_read_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_READ_STRING_EXT:
                _internalFuncNames.push_back("registry_read_string_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_SET_ROOT:
                _internalFuncNames.push_back("registry_set_root");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_WRITE_REAL:
                _internalFuncNames.push_back("registry_write_real");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_WRITE_REAL_EXT:
                _internalFuncNames.push_back("registry_write_real_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_WRITE_STRING:
                _internalFuncNames.push_back("registry_write_string");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case REGISTRY_WRITE_STRING_EXT:
                _internalFuncNames.push_back("registry_write_string_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_ADD:
                _internalFuncNames.push_back("room_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_ASSIGN:
                _internalFuncNames.push_back("room_assign");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_DUPLICATE:
                _internalFuncNames.push_back("room_duplicate");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_EXISTS:
                _internalFuncNames.push_back("room_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_GET_NAME:
                _internalFuncNames.push_back("room_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_GOTO:
                _internalFuncNames.push_back("room_goto");
                _gmlFuncs.push_back(&Runtime::room_goto);
                break;
            case ROOM_GOTO_NEXT:
                _internalFuncNames.push_back("room_goto_next");
                _gmlFuncs.push_back(&Runtime::room_goto_next);
                break;
            case ROOM_GOTO_PREVIOUS:
                _internalFuncNames.push_back("room_goto_previous");
                _gmlFuncs.push_back(&Runtime::room_goto_previous);
                break;
            case ROOM_INSTANCE_ADD:
                _internalFuncNames.push_back("room_instance_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_INSTANCE_CLEAR:
                _internalFuncNames.push_back("room_instance_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_NEXT:
                _internalFuncNames.push_back("room_next");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_PREVIOUS:
                _internalFuncNames.push_back("room_previous");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_RESTART:
                _internalFuncNames.push_back("room_restart");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_BACKGROUND:
                _internalFuncNames.push_back("room_set_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_BACKGROUND_COLOR:
                _internalFuncNames.push_back("room_set_background_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_CAPTION:
                _internalFuncNames.push_back("room_set_caption");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_CODE:
                _internalFuncNames.push_back("room_set_code");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_HEIGHT:
                _internalFuncNames.push_back("room_set_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_PERSISTENT:
                _internalFuncNames.push_back("room_set_persistent");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_VIEW:
                _internalFuncNames.push_back("room_set_view");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_VIEW_ENABLED:
                _internalFuncNames.push_back("room_set_view_enabled");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_SET_WIDTH:
                _internalFuncNames.push_back("room_set_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_TILE_ADD:
                _internalFuncNames.push_back("room_tile_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_TILE_ADD_EXT:
                _internalFuncNames.push_back("room_tile_add_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROOM_TILE_CLEAR:
                _internalFuncNames.push_back("room_tile_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case ROUND:
                _internalFuncNames.push_back("round");
                _gmlFuncs.push_back(&Runtime::round);
                break;
            case SCREEN_REDRAW:
                _internalFuncNames.push_back("screen_redraw");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCREEN_REFRESH:
                _internalFuncNames.push_back("screen_refresh");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCREEN_SAVE:
                _internalFuncNames.push_back("screen_save");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCREEN_SAVE_PART:
                _internalFuncNames.push_back("screen_save_part");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCREEN_WAIT_VSYNC:
                _internalFuncNames.push_back("screen_wait_vsync");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCRIPT_EXECUTE:
                _internalFuncNames.push_back("script_execute");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCRIPT_EXISTS:
                _internalFuncNames.push_back("script_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCRIPT_GET_NAME:
                _internalFuncNames.push_back("script_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SCRIPT_GET_TEXT:
                _internalFuncNames.push_back("script_get_text");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SET_APPLICATION_TITLE:
                _internalFuncNames.push_back("set_application_title");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SET_AUTOMATIC_DRAW:
                _internalFuncNames.push_back("set_automatic_draw");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SET_PROGRAM_PRIORITY:
                _internalFuncNames.push_back("set_program_priority");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SET_SYNCHRONIZATION:
                _internalFuncNames.push_back("set_synchronization");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_DEBUG_MESSAGE:
                _internalFuncNames.push_back("show_debug_message");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_ERROR:
                _internalFuncNames.push_back("show_error");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_INFO:
                _internalFuncNames.push_back("show_info");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_MENU:
                _internalFuncNames.push_back("show_menu");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_MENU_POS:
                _internalFuncNames.push_back("show_menu_pos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_MESSAGE:
                _internalFuncNames.push_back("show_message");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_MESSAGE_EXT:
                _internalFuncNames.push_back("show_message_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SHOW_QUESTION:
                _internalFuncNames.push_back("show_question");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SIGN:
                _internalFuncNames.push_back("sign");
                _gmlFuncs.push_back(&Runtime::sign);
                break;
            case SIN:
                _internalFuncNames.push_back("sin");
                _gmlFuncs.push_back(&Runtime::sin);
                break;
            case SLEEP:
                _internalFuncNames.push_back("sleep");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_3D_SET_SOUND_CONE:
                _internalFuncNames.push_back("sound_3d_set_sound_cone");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_3D_SET_SOUND_DISTANCE:
                _internalFuncNames.push_back("sound_3d_set_sound_distance");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_3D_SET_SOUND_POSITION:
                _internalFuncNames.push_back("sound_3d_set_sound_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_3D_SET_SOUND_VELOCITY:
                _internalFuncNames.push_back("sound_3d_set_sound_velocity");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_ADD:
                _internalFuncNames.push_back("sound_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_BACKGROUND_TEMPO:
                _internalFuncNames.push_back("sound_background_tempo");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_DELETE:
                _internalFuncNames.push_back("sound_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_DISCARD:
                _internalFuncNames.push_back("sound_discard");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_CHORUS:
                _internalFuncNames.push_back("sound_effect_chorus");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_COMPRESSOR:
                _internalFuncNames.push_back("sound_effect_compressor");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_ECHO:
                _internalFuncNames.push_back("sound_effect_echo");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_EQUALIZER:
                _internalFuncNames.push_back("sound_effect_equalizer");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_FLANGER:
                _internalFuncNames.push_back("sound_effect_flanger");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_GARGLE:
                _internalFuncNames.push_back("sound_effect_gargle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_REVERB:
                _internalFuncNames.push_back("sound_effect_reverb");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EFFECT_SET:
                _internalFuncNames.push_back("sound_effect_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_EXISTS:
                _internalFuncNames.push_back("sound_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_FADE:
                _internalFuncNames.push_back("sound_fade");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_GET_KIND:
                _internalFuncNames.push_back("sound_get_kind");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_GET_NAME:
                _internalFuncNames.push_back("sound_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_GET_PRELOAD:
                _internalFuncNames.push_back("sound_get_preload");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_GLOBAL_VOLUME:
                _internalFuncNames.push_back("sound_global_volume");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_ISPLAYING:
                _internalFuncNames.push_back("sound_isplaying");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_LOOP:
                _internalFuncNames.push_back("sound_loop");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_PAN:
                _internalFuncNames.push_back("sound_pan");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_PLAY:
                _internalFuncNames.push_back("sound_play");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_REPLACE:
                _internalFuncNames.push_back("sound_replace");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_RESTORE:
                _internalFuncNames.push_back("sound_restore");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_SET_SEARCH_DIRECTORY:
                _internalFuncNames.push_back("sound_set_search_directory");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_STOP:
                _internalFuncNames.push_back("sound_stop");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_STOP_ALL:
                _internalFuncNames.push_back("sound_stop_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SOUND_VOLUME:
                _internalFuncNames.push_back("sound_volume");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_ADAPT:
                _internalFuncNames.push_back("splash_set_adapt");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_BORDER:
                _internalFuncNames.push_back("splash_set_border");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_CAPTION:
                _internalFuncNames.push_back("splash_set_caption");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_CLOSE_BUTTON:
                _internalFuncNames.push_back("splash_set_close_button");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_COLOR:
                _internalFuncNames.push_back("splash_set_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_CURSOR:
                _internalFuncNames.push_back("splash_set_cursor");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_FULLSCREEN:
                _internalFuncNames.push_back("splash_set_fullscreen");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_INTERRUPT:
                _internalFuncNames.push_back("splash_set_interrupt");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_MAIN:
                _internalFuncNames.push_back("splash_set_main");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_POSITION:
                _internalFuncNames.push_back("splash_set_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_SCALE:
                _internalFuncNames.push_back("splash_set_scale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_SIZE:
                _internalFuncNames.push_back("splash_set_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_STOP_KEY:
                _internalFuncNames.push_back("splash_set_stop_key");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_STOP_MOUSE:
                _internalFuncNames.push_back("splash_set_stop_mouse");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SET_TOP:
                _internalFuncNames.push_back("splash_set_top");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SHOW_IMAGE:
                _internalFuncNames.push_back("splash_show_image");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SHOW_TEXT:
                _internalFuncNames.push_back("splash_show_text");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SHOW_VIDEO:
                _internalFuncNames.push_back("splash_show_video");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPLASH_SHOW_WEB:
                _internalFuncNames.push_back("splash_show_web");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_ADD:
                _internalFuncNames.push_back("sprite_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_ADD_FROM_SCREEN:
                _internalFuncNames.push_back("sprite_add_from_screen");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_ADD_FROM_SURFACE:
                _internalFuncNames.push_back("sprite_add_from_surface");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_ADD_SPRITE:
                _internalFuncNames.push_back("sprite_add_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_ASSIGN:
                _internalFuncNames.push_back("sprite_assign");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_COLLISION_MASK:
                _internalFuncNames.push_back("sprite_collision_mask");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_CREATE_FROM_SCREEN:
                _internalFuncNames.push_back("sprite_create_from_screen");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_CREATE_FROM_SURFACE:
                _internalFuncNames.push_back("sprite_create_from_surface");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_DELETE:
                _internalFuncNames.push_back("sprite_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_DUPLICATE:
                _internalFuncNames.push_back("sprite_duplicate");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_EXISTS:
                _internalFuncNames.push_back("sprite_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_BBOX_BOTTOM:
                _internalFuncNames.push_back("sprite_get_bbox_bottom");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_BBOX_LEFT:
                _internalFuncNames.push_back("sprite_get_bbox_left");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_BBOX_MODE:
                _internalFuncNames.push_back("sprite_get_bbox_mode");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_BBOX_RIGHT:
                _internalFuncNames.push_back("sprite_get_bbox_right");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_BBOX_TOP:
                _internalFuncNames.push_back("sprite_get_bbox_top");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_HEIGHT:
                _internalFuncNames.push_back("sprite_get_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_NAME:
                _internalFuncNames.push_back("sprite_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_NUMBER:
                _internalFuncNames.push_back("sprite_get_number");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_PRECISE:
                _internalFuncNames.push_back("sprite_get_precise");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_PRELOAD:
                _internalFuncNames.push_back("sprite_get_preload");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_SMOOTH:
                _internalFuncNames.push_back("sprite_get_smooth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_TEXTURE:
                _internalFuncNames.push_back("sprite_get_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_WIDTH:
                _internalFuncNames.push_back("sprite_get_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_XOFFSET:
                _internalFuncNames.push_back("sprite_get_xoffset");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_GET_YOFFSET:
                _internalFuncNames.push_back("sprite_get_yoffset");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_MERGE:
                _internalFuncNames.push_back("sprite_merge");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_REPLACE:
                _internalFuncNames.push_back("sprite_replace");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_REPLACE_SPRITE:
                _internalFuncNames.push_back("sprite_replace_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_SAVE:
                _internalFuncNames.push_back("sprite_save");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_SAVE_STRIP:
                _internalFuncNames.push_back("sprite_save_strip");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_SET_ALPHA_FROM_SPRITE:
                _internalFuncNames.push_back("sprite_set_alpha_from_sprite");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SPRITE_SET_OFFSET:
                _internalFuncNames.push_back("sprite_set_offset");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SQR:
                _internalFuncNames.push_back("sqr");
                _gmlFuncs.push_back(&Runtime::sqr);
                break;
            case SQRT:
                _internalFuncNames.push_back("sqrt");
                _gmlFuncs.push_back(&Runtime::sqrt);
                break;
            case STRING:
                _internalFuncNames.push_back("string");
                _gmlFuncs.push_back(&Runtime::string);
                break;
            case STRING_BYTE_AT:
                _internalFuncNames.push_back("string_byte_at");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_BYTE_LENGTH:
                _internalFuncNames.push_back("string_byte_length");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_CHAR_AT:
                _internalFuncNames.push_back("string_char_at");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_COPY:
                _internalFuncNames.push_back("string_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_COUNT:
                _internalFuncNames.push_back("string_count");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_DELETE:
                _internalFuncNames.push_back("string_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_DIGITS:
                _internalFuncNames.push_back("string_digits");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_FORMAT:
                _internalFuncNames.push_back("string_format");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_HEIGHT:
                _internalFuncNames.push_back("string_height");
                _gmlFuncs.push_back(&Runtime::string_height);
                break;
            case STRING_HEIGHT_EXT:
                _internalFuncNames.push_back("string_height_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_INSERT:
                _internalFuncNames.push_back("string_insert");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_LENGTH:
                _internalFuncNames.push_back("string_length");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_LETTERS:
                _internalFuncNames.push_back("string_letters");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_LETTERSDIGITS:
                _internalFuncNames.push_back("string_lettersdigits");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_LOWER:
                _internalFuncNames.push_back("string_lower");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_POS:
                _internalFuncNames.push_back("string_pos");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_REPEAT:
                _internalFuncNames.push_back("string_repeat");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_REPLACE:
                _internalFuncNames.push_back("string_replace");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_REPLACE_ALL:
                _internalFuncNames.push_back("string_replace_all");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_UPPER:
                _internalFuncNames.push_back("string_upper");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case STRING_WIDTH:
                _internalFuncNames.push_back("string_width");
                _gmlFuncs.push_back(&Runtime::string_width);
                break;
            case STRING_WIDTH_EXT:
                _internalFuncNames.push_back("string_width_ext");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_COPY:
                _internalFuncNames.push_back("surface_copy");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_COPY_PART:
                _internalFuncNames.push_back("surface_copy_part");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_CREATE:
                _internalFuncNames.push_back("surface_create");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_EXISTS:
                _internalFuncNames.push_back("surface_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_FREE:
                _internalFuncNames.push_back("surface_free");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_GET_HEIGHT:
                _internalFuncNames.push_back("surface_get_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_GET_TEXTURE:
                _internalFuncNames.push_back("surface_get_texture");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_GET_WIDTH:
                _internalFuncNames.push_back("surface_get_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_GETPIXEL:
                _internalFuncNames.push_back("surface_getpixel");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_RESET_TARGET:
                _internalFuncNames.push_back("surface_reset_target");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_SAVE:
                _internalFuncNames.push_back("surface_save");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_SAVE_PART:
                _internalFuncNames.push_back("surface_save_part");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case SURFACE_SET_TARGET:
                _internalFuncNames.push_back("surface_set_target");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TAN:
                _internalFuncNames.push_back("tan");
                _gmlFuncs.push_back(&Runtime::tan);
                break;
            case TEXTURE_GET_HEIGHT:
                _internalFuncNames.push_back("texture_get_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TEXTURE_GET_WIDTH:
                _internalFuncNames.push_back("texture_get_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TEXTURE_PRELOAD:
                _internalFuncNames.push_back("texture_preload");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TEXTURE_SET_BLENDING:
                _internalFuncNames.push_back("texture_set_blending");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TEXTURE_SET_INTERPOLATION:
                _internalFuncNames.push_back("texture_set_interpolation");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TEXTURE_SET_PRIORITY:
                _internalFuncNames.push_back("texture_set_priority");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TEXTURE_SET_REPEAT:
                _internalFuncNames.push_back("texture_set_repeat");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_ADD:
                _internalFuncNames.push_back("tile_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_DELETE:
                _internalFuncNames.push_back("tile_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_EXISTS:
                _internalFuncNames.push_back("tile_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_ALPHA:
                _internalFuncNames.push_back("tile_get_alpha");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_BACKGROUND:
                _internalFuncNames.push_back("tile_get_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_BLEND:
                _internalFuncNames.push_back("tile_get_blend");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_DEPTH:
                _internalFuncNames.push_back("tile_get_depth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_HEIGHT:
                _internalFuncNames.push_back("tile_get_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_LEFT:
                _internalFuncNames.push_back("tile_get_left");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_TOP:
                _internalFuncNames.push_back("tile_get_top");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_VISIBLE:
                _internalFuncNames.push_back("tile_get_visible");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_WIDTH:
                _internalFuncNames.push_back("tile_get_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_X:
                _internalFuncNames.push_back("tile_get_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_XSCALE:
                _internalFuncNames.push_back("tile_get_xscale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_Y:
                _internalFuncNames.push_back("tile_get_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_GET_YSCALE:
                _internalFuncNames.push_back("tile_get_yscale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_LAYER_DELETE:
                _internalFuncNames.push_back("tile_layer_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_LAYER_DELETE_AT:
                _internalFuncNames.push_back("tile_layer_delete_at");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_LAYER_DEPTH:
                _internalFuncNames.push_back("tile_layer_depth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_LAYER_FIND:
                _internalFuncNames.push_back("tile_layer_find");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_LAYER_HIDE:
                _internalFuncNames.push_back("tile_layer_hide");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_LAYER_SHIFT:
                _internalFuncNames.push_back("tile_layer_shift");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_LAYER_SHOW:
                _internalFuncNames.push_back("tile_layer_show");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_ALPHA:
                _internalFuncNames.push_back("tile_set_alpha");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_BACKGROUND:
                _internalFuncNames.push_back("tile_set_background");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_BLEND:
                _internalFuncNames.push_back("tile_set_blend");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_DEPTH:
                _internalFuncNames.push_back("tile_set_depth");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_POSITION:
                _internalFuncNames.push_back("tile_set_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_REGION:
                _internalFuncNames.push_back("tile_set_region");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_SCALE:
                _internalFuncNames.push_back("tile_set_scale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TILE_SET_VISIBLE:
                _internalFuncNames.push_back("tile_set_visible");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TIMELINE_ADD:
                _internalFuncNames.push_back("timeline_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TIMELINE_CLEAR:
                _internalFuncNames.push_back("timeline_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TIMELINE_DELETE:
                _internalFuncNames.push_back("timeline_delete");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TIMELINE_EXISTS:
                _internalFuncNames.push_back("timeline_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TIMELINE_GET_NAME:
                _internalFuncNames.push_back("timeline_get_name");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TIMELINE_MOMENT_ADD:
                _internalFuncNames.push_back("timeline_moment_add");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TIMELINE_MOMENT_CLEAR:
                _internalFuncNames.push_back("timeline_moment_clear");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TRANSITION_DEFINE:
                _internalFuncNames.push_back("transition_define");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case TRANSITION_EXISTS:
                _internalFuncNames.push_back("transition_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_GLOBAL_ARRAY_GET:
                _internalFuncNames.push_back("variable_global_array_get");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_GLOBAL_ARRAY_SET:
                _internalFuncNames.push_back("variable_global_array_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_GLOBAL_ARRAY2_GET:
                _internalFuncNames.push_back("variable_global_array2_get");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_GLOBAL_ARRAY2_SET:
                _internalFuncNames.push_back("variable_global_array2_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_GLOBAL_EXISTS:
                _internalFuncNames.push_back("variable_global_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_GLOBAL_GET:
                _internalFuncNames.push_back("variable_global_get");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_GLOBAL_SET:
                _internalFuncNames.push_back("variable_global_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_LOCAL_ARRAY_GET:
                _internalFuncNames.push_back("variable_local_array_get");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_LOCAL_ARRAY_SET:
                _internalFuncNames.push_back("variable_local_array_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_LOCAL_ARRAY2_GET:
                _internalFuncNames.push_back("variable_local_array2_get");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_LOCAL_ARRAY2_SET:
                _internalFuncNames.push_back("variable_local_array2_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_LOCAL_EXISTS:
                _internalFuncNames.push_back("variable_local_exists");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_LOCAL_GET:
                _internalFuncNames.push_back("variable_local_get");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case VARIABLE_LOCAL_SET:
                _internalFuncNames.push_back("variable_local_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_CENTER:
                _internalFuncNames.push_back("window_center");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_DEFAULT:
                _internalFuncNames.push_back("window_default");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_CAPTION:
                _internalFuncNames.push_back("window_get_caption");
                _gmlFuncs.push_back(&Runtime::window_get_caption);
                break;
            case WINDOW_GET_COLOR:
                _internalFuncNames.push_back("window_get_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_CURSOR:
                _internalFuncNames.push_back("window_get_cursor");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_FULLSCREEN:
                _internalFuncNames.push_back("window_get_fullscreen");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_HEIGHT:
                _internalFuncNames.push_back("window_get_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_REGION_HEIGHT:
                _internalFuncNames.push_back("window_get_region_height");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_REGION_SCALE:
                _internalFuncNames.push_back("window_get_region_scale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_REGION_WIDTH:
                _internalFuncNames.push_back("window_get_region_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_SHOWBORDER:
                _internalFuncNames.push_back("window_get_showborder");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_SHOWICONS:
                _internalFuncNames.push_back("window_get_showicons");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_SIZEABLE:
                _internalFuncNames.push_back("window_get_sizeable");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_STAYONTOP:
                _internalFuncNames.push_back("window_get_stayontop");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_VISIBLE:
                _internalFuncNames.push_back("window_get_visible");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_WIDTH:
                _internalFuncNames.push_back("window_get_width");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_X:
                _internalFuncNames.push_back("window_get_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_GET_Y:
                _internalFuncNames.push_back("window_get_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_HANDLE:
                _internalFuncNames.push_back("window_handle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_MOUSE_GET_X:
                _internalFuncNames.push_back("window_mouse_get_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_MOUSE_GET_Y:
                _internalFuncNames.push_back("window_mouse_get_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_MOUSE_SET:
                _internalFuncNames.push_back("window_mouse_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_CAPTION:
                _internalFuncNames.push_back("window_set_caption");
                _gmlFuncs.push_back(&Runtime::window_set_caption);
                break;
            case WINDOW_SET_COLOR:
                _internalFuncNames.push_back("window_set_color");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_CURSOR:
                _internalFuncNames.push_back("window_set_cursor");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_FULLSCREEN:
                _internalFuncNames.push_back("window_set_fullscreen");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_POSITION:
                _internalFuncNames.push_back("window_set_position");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_RECTANGLE:
                _internalFuncNames.push_back("window_set_rectangle");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_REGION_SCALE:
                _internalFuncNames.push_back("window_set_region_scale");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_REGION_SIZE:
                _internalFuncNames.push_back("window_set_region_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_SHOWBORDER:
                _internalFuncNames.push_back("window_set_showborder");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_SHOWICONS:
                _internalFuncNames.push_back("window_set_showicons");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_SIZE:
                _internalFuncNames.push_back("window_set_size");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_SIZEABLE:
                _internalFuncNames.push_back("window_set_sizeable");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_STAYONTOP:
                _internalFuncNames.push_back("window_set_stayontop");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_SET_VISIBLE:
                _internalFuncNames.push_back("window_set_visible");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_VIEW_MOUSE_GET_X:
                _internalFuncNames.push_back("window_view_mouse_get_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_VIEW_MOUSE_GET_Y:
                _internalFuncNames.push_back("window_view_mouse_get_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_VIEW_MOUSE_SET:
                _internalFuncNames.push_back("window_view_mouse_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_VIEWS_MOUSE_GET_X:
                _internalFuncNames.push_back("window_views_mouse_get_x");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_VIEWS_MOUSE_GET_Y:
                _internalFuncNames.push_back("window_views_mouse_get_y");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            case WINDOW_VIEWS_MOUSE_SET:
                _internalFuncNames.push_back("window_views_mouse_set");
                _gmlFuncs.push_back(&Runtime::unimplemented);
                break;
            default:
                // There's something in the enum that isn't listed here. Abort
                return false;
        }
    }
    Runtime::Init(globals, _gmlFuncs);
    return true;
}

unsigned int GM8Emulator::Compiler::_RegisterField(const std::string_view& name) {
    for (unsigned int i = 0; i < _fieldNames.size(); i++) {
        if (name == _fieldNames[i]) {
            return i;
        }
    }
    _fieldNames.push_back(std::string(name));
    return ( unsigned int )(_fieldNames.size() - 1);
}

std::set<unsigned int> _locals;
void GM8Emulator::Compiler::FlushLocals() { _locals.clear(); }

bool GM8Emulator::Compiler::Interpret(const TokenList& list, CRActionList* output) {
    unsigned int pos = 0;
    CRAction* action;
    while (pos < list.tokens.size()) {
        if (!_InterpretLine(list, &action, pos)) {
            return false;
        }
        output->Append(action);
    }
    return true;
}

bool GM8Emulator::Compiler::InterpretExpression(const TokenList& list, CRExpression* output, unsigned int* pos, char precedence) {
    // pos can be nullptr
    unsigned int p = 0;
    if (!pos) pos = &p;

    // Read expression values iteratively
    while (true) {
        // Read any unary operators
        std::vector<CRUnaryOperator> unaries;
        while (list.tokens[*pos].type == Token::token_type::Operator) {
            switch (list.tokens[*pos].value.op) {
                case OperatorType::Add:
                    // unaries.push_back(CRUnaryOperator::OPERATOR_POSITIVE); // This doesn't do anything so no reason to compile it
                    break;
                case OperatorType::Subtract:
                    unaries.push_back(CRUnaryOperator::OPERATOR_NEGATIVE);
                    break;
                case OperatorType::Complement:
                    unaries.push_back(CRUnaryOperator::OPERATOR_TILDE);
                    break;
                case OperatorType::Not:
                    unaries.push_back(CRUnaryOperator::OPERATOR_NOT);
                    break;
                default:
                    return false;
            }
            (*pos)++;
        }

        // Read the deref for this expression
        GM8Emulator::Compiler::TokenList deref;
        // unsigned int storedPos = *pos;
        unsigned int newPos = *pos;
        while (true) {
            if (_TokenHasValue(list.tokens[newPos], SeparatorType::ParenLeft)) {
                unsigned int depth = 1;
                while (++newPos) {
                    // deref.push_back(list[newPos]);
                    if (_TokenHasValue(list.tokens[newPos], SeparatorType::ParenLeft))
                        depth++;
                    else if (_TokenHasValue(list.tokens[newPos], SeparatorType::ParenRight)) {
                        depth--;
                        if (depth == 0) {
                            newPos++;
                            break;
                        }
                    }
                }
            }
            else if (list.tokens[newPos].type == Token::token_type::Identifier) {
                newPos++;
                if (newPos < list.tokens.size()) {
                    if (_TokenHasValue(list.tokens[newPos], SeparatorType::SquareBracketLeft)) {
                        // Skip over array accessor
                        unsigned int depth = 1;
                        while (++newPos) {
                            if (_TokenHasValue(list.tokens[newPos], SeparatorType::SquareBracketLeft))
                                depth++;
                            else if (_TokenHasValue(list.tokens[newPos], SeparatorType::SquareBracketRight)) {
                                depth--;
                                if (depth == 0) {
                                    newPos++;
                                    break;
                                }
                            }
                        }
                    }
                    else if (_TokenHasValue(list.tokens[newPos], SeparatorType::ParenLeft)) {
                        // Skip over function args
                        unsigned int depth = 1;
                        while (++newPos) {
                            if (_TokenHasValue(list.tokens[newPos], SeparatorType::ParenLeft))
                                depth++;
                            else if (_TokenHasValue(list.tokens[newPos], SeparatorType::ParenRight)) {
                                depth--;
                                if (depth == 0) {
                                    newPos++;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (newPos < list.tokens.size()) {
                if (_TokenHasValue(list.tokens[newPos], SeparatorType::Period)) {
                    if (deref.tokens.size()) deref.tokens.push_back(list.tokens[newPos]);  // Add a period to the deref expression if it's already populated
                    for (unsigned int i = *pos; i < newPos; i++) {
                        deref.tokens.push_back(list.tokens[i]);
                    }
                    newPos++;
                    (*pos) = newPos;
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }
        //(*pos) = storedPos;

        CRExpression derefCompiled;
        bool hasDeref = false;
        if (deref.tokens.size()) {
            hasDeref = true;
            if (!InterpretExpression(deref, &derefCompiled)) return false;
        }

        // Read one expression variable
        CRExpressionValue* value;
        switch (list.tokens[*pos].type) {

            case Token::token_type::Separator: {
                if (hasDeref) return false;  // Fail if there is a deref before this
                if (list.tokens[*pos].value.sep != SeparatorType::ParenLeft) return false;
                TokenList innerList;
                unsigned int depth = 1;
                while (++(*pos)) {
                    if (_TokenHasValue(list.tokens[*pos], SeparatorType::ParenLeft)) depth++;
                    if (_TokenHasValue(list.tokens[*pos], SeparatorType::ParenRight)) {
                        depth--;
                        if (!depth) break;
                    }
                    innerList.tokens.push_back(list.tokens[*pos]);
                }
                (*pos)++;
                CRExpression inner;
                if (!InterpretExpression(innerList, &inner)) return false;
                if (inner.GetValues()->size() == 1) {
                    value = (*inner.GetValues())[0];
                }
                else {
                    value = new CRExpNestedExpression(inner);
                }
                break;
            }

            case Token::token_type::Identifier: {
                // Store identifier name for later
                std::string_view identifier = list.tokens[*pos].value.str;
                (*pos)++;

                if ((*pos) < list.tokens.size() && _TokenHasValue(list.tokens[*pos], SeparatorType::ParenLeft)) {
                    // This is a script or function
                    (*pos)++;
                    // Parse args
                    std::vector<CRExpression> args;
                    while (!_TokenHasValue(list.tokens[*pos], SeparatorType::ParenRight)) {
                        CRExpression arg;
                        if (!InterpretExpression(list, &arg, pos)) return false;
                        args.push_back(arg);
                        if (_TokenHasValue(list.tokens[*pos], SeparatorType::Comma)) (*pos)++;
                    }
                    (*pos)++;

                    // Check if this is a user script (scripts have precedence over internal functions)
                    unsigned int scriptCount = AssetManager::GetScriptCount();
                    unsigned int s;
                    for (s = 0; s < scriptCount; s++) {
                        Script* scr = AssetManager::GetScript(s);
                        if (!scr->exists) continue;
                        if (identifier.compare(scr->name) == 0) {
                            value = new CRExpScript(s, args);
                            break;
                        }
                    }

                    if (s == scriptCount) {
                        // Check if this is an internal function
                        unsigned int f;
                        for (f = 0; f < _internalFuncNames.size(); f++) {
                            if (identifier.compare(_internalFuncNames[f]) == 0) {
                                value = new CRExpFunction(( CRInternalFunction )f, args);
                                break;
                            }
                        }

                        // Unrecognized function name?
                        if (f == _internalFuncNames.size()) return false;
                    }
                }
                else {
                    // Parse array accessor
                    std::vector<CRExpression> dimensions;
                    if ((*pos) < list.tokens.size()) {
                        if (_TokenHasValue(list.tokens[*pos], SeparatorType::SquareBracketLeft)) {
                            (*pos)++;
                            while (!_TokenHasValue(list.tokens[*pos], SeparatorType::SquareBracketRight)) {
                                CRExpression dim;
                                if (!InterpretExpression(list, &dim, pos)) return false;
                                dimensions.push_back(dim);
                                if (_TokenHasValue(list.tokens[*pos], SeparatorType::Comma)) {
                                    (*pos)++;
                                }
                                else if (_TokenHasValue(list.tokens[*pos], SeparatorType::SquareBracketRight)) {
                                    (*pos)++;
                                    break;
                                }
                                else
                                    return false;
                            }
                        }
                    }

                    // Order of priority: assets, gml consts, fields
                    unsigned int assetId;
                    if (_IsAsset(identifier, &assetId)) {
                        if (hasDeref) return false;
                        value = new CRExpLiteral(static_cast<double>(assetId));
                    }
                    else {
                        double constValue;
                        if (_IsGMLConst(identifier, &constValue)) {
                            if (hasDeref) return false;
                            value = new CRExpLiteral(constValue);
                        }
                        else {
                            CRGameVar gameVar;
                            if (_IsGameValue(identifier, &gameVar)) {
                                value = new CRExpGameVar(gameVar, dimensions);
                            }
                            else {
                                CRInstanceVar instanceVar;
                                if (_IsInstanceVar(identifier, &instanceVar)) {
                                    if (hasDeref)
                                        value = new CRExpInstanceVar(instanceVar, dimensions, derefCompiled);
                                    else
                                        value = new CRExpInstanceVar(instanceVar, dimensions);
                                }
                                else {
                                    unsigned int fieldNumber = _RegisterField(identifier);
                                    bool isLocal = _locals.count(fieldNumber);
                                    if (!dimensions.size()) {
                                        if (hasDeref)
                                            value = new CRExpField(fieldNumber, derefCompiled, isLocal);
                                        else
                                            value = new CRExpField(fieldNumber, isLocal);
                                    }
                                    else {
                                        if (hasDeref)
                                            value = new CRExpArray(fieldNumber, dimensions, derefCompiled, isLocal);
                                        else
                                            value = new CRExpArray(fieldNumber, dimensions, isLocal);
                                    }
                                }
                            }
                        }
                    }
                }

                break;
            }

            case Token::token_type::Real: {
                if (hasDeref) return false;  // Fail if there is a deref before this
                value = new CRExpLiteral(list.tokens[*pos].value.real);
                (*pos)++;
                break;
            }

            case Token::token_type::String: {
                if (hasDeref) return false;  // Fail if there is a deref before this
                value = new CRExpLiteral(std::string(list.tokens[*pos].value.str));
                (*pos)++;
                break;
            }
            default:
                return false;
        }
        if (value->GetUnaries()->size()) {
            std::copy(value->GetUnaries()->begin(), value->GetUnaries()->end(), std::back_inserter(unaries));
        }

        // Reverse unary operator list before adding it to value
        std::reverse(unaries.begin(), unaries.end());
        value->SetUnaries(std::move(unaries));

        // Parse an operator
        if ((*pos) >= list.tokens.size()) {
            // No operators here because it's the end of the code, so exit
            value->SetOperator(CROperator::OPERATOR_NONE);
            output->Append(value);
            return true;
        }
        if (list.tokens[*pos].type == Token::token_type::Operator) {
            switch (list.tokens[*pos].value.op) {
                case OperatorType::Add:
                    value->SetOperator(CROperator::OPERATOR_ADD);
                    break;
                case OperatorType::And:
                    value->SetOperator(CROperator::OPERATOR_BOOLEAN_AND);
                    break;
                case OperatorType::Assign:
                case OperatorType::Equal:
                    value->SetOperator(CROperator::OPERATOR_EQUALS);
                    break;
                case OperatorType::BinaryAnd:
                    value->SetOperator(CROperator::OPERATOR_BITWISE_AND);
                    break;
                case OperatorType::BinaryOr:
                    value->SetOperator(CROperator::OPERATOR_BITWISE_OR);
                    break;
                case OperatorType::BinaryShiftLeft:
                    value->SetOperator(CROperator::OPERATOR_LSHIFT);
                    break;
                case OperatorType::BinaryShiftRight:
                    value->SetOperator(CROperator::OPERATOR_RSHIFT);
                    break;
                case OperatorType::BinaryXor:
                    value->SetOperator(CROperator::OPERATOR_BITWISE_XOR);
                    break;
                case OperatorType::Divide:
                    value->SetOperator(CROperator::OPERATOR_DIVIDE);
                    break;
                case OperatorType::DivideAndFloor:
                    value->SetOperator(CROperator::OPERATOR_DIV);
                    break;
                case OperatorType::GreaterThan:
                    value->SetOperator(CROperator::OPERATOR_GT);
                    break;
                case OperatorType::GreaterThanOrEquals:
                    value->SetOperator(CROperator::OPERATOR_GTE);
                    break;
                case OperatorType::LessThan:
                    value->SetOperator(CROperator::OPERATOR_LT);
                    break;
                case OperatorType::LessThanOrEquals:
                    value->SetOperator(CROperator::OPERATOR_LTE);
                    break;
                case OperatorType::Modulo:
                    value->SetOperator(CROperator::OPERATOR_MOD);
                    break;
                case OperatorType::Multiply:
                    value->SetOperator(CROperator::OPERATOR_MULTIPLY);
                    break;
                case OperatorType::NotEqual:
                    value->SetOperator(CROperator::OPERATOR_NOT_EQUAL);
                    break;
                case OperatorType::Or:
                    value->SetOperator(CROperator::OPERATOR_BOOLEAN_OR);
                    break;
                case OperatorType::Subtract:
                    value->SetOperator(CROperator::OPERATOR_SUBTRACT);
                    break;
                case OperatorType::Xor:
                    value->SetOperator(CROperator::OPERATOR_BOOLEAN_XOR);
                    break;
                default:
                    return false;
            }
            (*pos)++;

            char newPrec = _operatorPrecedence[value->GetOperator()];
            if (newPrec > precedence) {
                CRExpression newNested;
                newNested.Append(value);
                if (!InterpretExpression(list, &newNested, pos, newPrec)) return false;
                output->Append(new CRExpNestedExpression(newNested));
                return true;
            }
            else {
                output->Append(value);
                precedence = newPrec;
            }
        }
        else {
            value->SetOperator(CROperator::OPERATOR_NONE);
            output->Append(value);
            return true;
        }
    }
}


bool GM8Emulator::Compiler::_InterpretLine(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    switch (list.tokens[pos].type) {

        case Token::token_type::Keyword:
            switch (list.tokens[pos].value.key) {
                case KeywordType::Var:
                    return _InterpretVar(list, output, pos);
                case KeywordType::If:
                    return _InterpretIfElse(list, output, pos);
                case KeywordType::With:
                    return _InterpretWith(list, output, pos);
                case KeywordType::Repeat:
                    return _InterpretRepeat(list, output, pos);
                case KeywordType::Do:
                    return _InterpretDoUntil(list, output, pos);
                case KeywordType::While:
                    return _InterpretWhile(list, output, pos);
                case KeywordType::For:
                    return _InterpretFor(list, output, pos);
                case KeywordType::Switch:
                    return _InterpretSwitch(list, output, pos);
                case KeywordType::Break:
                    return _InterpretBreak(list, output, pos);
                case KeywordType::Continue:
                    return _InterpretContinue(list, output, pos);
                case KeywordType::Return:
                    return _InterpretReturn(list, output, pos);
                case KeywordType::Exit:
                    return _InterpretExit(list, output, pos);
                default:
                    return false;
            }

        case Token::token_type::Identifier:
            if (pos + 1 >= list.tokens.size()) return false;
            if (list.tokens[pos + 1].type == Token::token_type::Separator) {
                if (list.tokens[pos + 1].value.sep == SeparatorType::ParenLeft) {
                    return _InterpretFunction(list, output, pos);
                }
            }
            return _InterpretAssignment(list, output, pos);

        case Token::token_type::Separator:
            switch (list.tokens[pos].value.sep) {
                case SeparatorType::ParenLeft:
                    return _InterpretAssignment(list, output, pos);
                case SeparatorType::BraceLeft:
                    return _InterpretBraces(list, output, pos);
                default:
                    return false;
            }
        default:
            return false;
    }
}


bool GM8Emulator::Compiler::_InterpretVar(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    if (list.tokens[pos].type != Token::token_type::Identifier) return false;
    _locals.insert(_RegisterField(list.tokens[pos++].value.str));
    while (_TokenHasValue(list.tokens[pos], SeparatorType::Comma)) {
        pos++;
        if (list.tokens[pos].type != Token::token_type::Identifier) return false;
        _locals.insert(_RegisterField(list.tokens[pos++].value.str));
    }
    _SkipSemicolon(list, pos);
    (*output) = new CRActionBindVars();
    return true;
}

bool GM8Emulator::Compiler::_InterpretIfElse(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    CRExpression condition;
    CRAction* aIf;
    CRAction* aElse = nullptr;
    if (!InterpretExpression(list, &condition, &pos)) return false;
    if (_TokenHasValue(list.tokens[pos], SeparatorType::PascalThen)) pos++;
    if (!_InterpretLine(list, &aIf, pos)) return false;
    if (pos < list.tokens.size()) {
        if (_TokenHasValue(list.tokens[pos], KeywordType::Else)) {
            pos++;
            if (!_InterpretLine(list, &aElse, pos)) return false;
        }
    }
    (*output) = new CRActionIfElse(aIf, aElse, condition);
    return true;
}

bool GM8Emulator::Compiler::_InterpretWith(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    CRExpression exp;
    if (!InterpretExpression(list, &exp, &pos)) return false;
    CRAction* action;
    if (!_InterpretLine(list, &action, pos)) return false;
    (*output) = new CRActionWith(exp, action);
    return true;
}

bool GM8Emulator::Compiler::_InterpretRepeat(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    CRExpression exp;
    if (!InterpretExpression(list, &exp, &pos)) return false;
    CRAction* action;
    if (!_InterpretLine(list, &action, pos)) return false;
    (*output) = new CRActionRepeat(exp, action);
    return true;
}

bool GM8Emulator::Compiler::_InterpretDoUntil(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    CRAction* action;
    if (!_InterpretLine(list, &action, pos)) return false;
    if (!_TokenHasValue(list.tokens[pos], KeywordType::Until)) return false;
    pos++;
    CRExpression exp;
    if (!InterpretExpression(list, &exp, &pos)) return false;
    (*output) = new CRActionDoUntil(exp, action);
    _SkipSemicolon(list, pos);
    return true;
}

bool GM8Emulator::Compiler::_InterpretWhile(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    CRExpression exp;
    if (!InterpretExpression(list, &exp, &pos)) return false;
    CRAction* action;
    if (!_InterpretLine(list, &action, pos)) return false;
    (*output) = new CRActionWhile(exp, action);
    return true;
}

bool GM8Emulator::Compiler::_InterpretFor(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    if (!_TokenHasValue(list.tokens[pos], SeparatorType::ParenLeft)) return false;
    pos++;
    CRAction* init;
    if (!_InterpretLine(list, &init, pos)) return false;
    CRExpression exp;
    if (!InterpretExpression(list, &exp, &pos)) return false;
    if (!_TokenHasValue(list.tokens[pos], SeparatorType::Semicolon)) return false;
    pos++;
    CRAction* final;
    if (!_InterpretLine(list, &final, pos)) return false;
    if (!_TokenHasValue(list.tokens[pos], SeparatorType::ParenRight)) return false;
    pos++;
    CRAction* code;
    if (!_InterpretLine(list, &code, pos)) return false;
    (*output) = new CRActionFor(init, exp, final, code);
    return true;
}

bool GM8Emulator::Compiler::_InterpretSwitch(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    pos++;
    CRExpression exp;
    if (!InterpretExpression(list, &exp, &pos)) return false;
    if (!_TokenHasValue(list.tokens[pos], SeparatorType::BraceLeft)) return false;
    pos++;

    bool defaulted = false;  // have we already found the default keyword
    unsigned int defaultOffset;
    std::vector<SwitchCase> cases;
    CRActionList actions;
    while (true) {
        if (_TokenHasValue(list.tokens[pos], SeparatorType::BraceRight))
            break;
        else if (_TokenHasValue(list.tokens[pos], KeywordType::Case)) {
            pos++;
            CRExpression c;
            if (!InterpretExpression(list, &c, &pos)) return false;
            if (!_TokenHasValue(list.tokens[pos], SeparatorType::Colon)) return false;
            pos++;
            if (!defaulted) cases.push_back(SwitchCase(c, ( unsigned int )actions.Count()));
        }
        else if (_TokenHasValue(list.tokens[pos], KeywordType::Default)) {
            defaulted = true;
            pos++;
            if (!_TokenHasValue(list.tokens[pos], SeparatorType::Colon)) return false;
            pos++;
            defaultOffset = ( unsigned int )actions.Count();
        }
        else {
            CRAction* act;
            if (!_InterpretLine(list, &act, pos)) return false;
            actions.Append(act);
        }
    }
    pos++;
    if (!defaulted) defaultOffset = ( unsigned int )actions.Count();
    (*output) = new CRActionSwitch(exp, actions, cases, defaultOffset);
    return true;
}

bool GM8Emulator::Compiler::_InterpretBreak(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    (*output) = new CRActionBreak();
    pos++;
    _SkipSemicolon(list, pos);
    return true;
}

bool GM8Emulator::Compiler::_InterpretContinue(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    (*output) = new CRActionContinue();
    pos++;
    _SkipSemicolon(list, pos);
    return true;
}

bool GM8Emulator::Compiler::_InterpretReturn(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    CRExpression exp;
    pos++;
    if (!InterpretExpression(list, &exp, &pos)) return false;
    (*output) = new CRActionReturn(exp);
    _SkipSemicolon(list, pos);
    return true;
}

bool GM8Emulator::Compiler::_InterpretExit(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    (*output) = new CRActionExit();
    pos++;
    _SkipSemicolon(list, pos);
    return true;
}

bool GM8Emulator::Compiler::_InterpretAssignment(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    // The first thing to interpret is the dereference expression. There may or may not be one.
    GM8Emulator::Compiler::TokenList deref;

    if (_TokenHasValue(list.tokens[pos], SeparatorType::ParenLeft)) {
        deref.tokens.push_back(list.tokens[pos]);
        // If the line starts with an open-paren, it's definitely a deref. This format is only allowed at the start of the line.
        unsigned int depth = 1;
        while (++pos) {
            deref.tokens.push_back(list.tokens[pos]);
            if (_TokenHasValue(list.tokens[pos], SeparatorType::ParenLeft)) {
                depth++;
            }
            else if (_TokenHasValue(list.tokens[pos], SeparatorType::ParenRight)) {
                depth--;
                if (depth == 0) {
                    pos++;
                    break;
                }
            }
        }
        if (!_TokenHasValue(list.tokens[pos], SeparatorType::Period)) return false;
        pos++;
    }

    while (true) {
        if (list.tokens[pos].type != Token::token_type::Identifier) return false;

        // This token will indicate a var. Now, we need to work out if there's a period after this variable indication. If so, it's part of the deref.
        unsigned int startPos = pos;
        pos++;
        if (_TokenHasValue(list.tokens[pos], SeparatorType::SquareBracketLeft)) {
            // Skip over an array accessor...
            unsigned int depth = 1;
            while (++pos) {
                if (_TokenHasValue(list.tokens[pos], SeparatorType::SquareBracketLeft))
                    depth++;
                else if (_TokenHasValue(list.tokens[pos], SeparatorType::SquareBracketRight)) {
                    depth--;
                    if (!depth) {
                        pos++;
                        break;
                    }
                }
            }
        }

        if (_TokenHasValue(list.tokens[pos], SeparatorType::Period)) {
            if (deref.tokens.size()) deref.tokens.push_back(list.tokens[pos]);  // Add a period to the deref if it's already got something in it
            for (unsigned int i = startPos; i < pos; i++) {
                deref.tokens.push_back(list.tokens[i]);
            }
            pos++;
        }
        else {
            pos = startPos;
            break;
        }
    }

    // Now we're past the dereference part, so compile it
    bool hasDeref = deref.tokens.size();
    CRExpression derefCompiled;
    if (hasDeref) {
        unsigned int p = 0;
        if (!InterpretExpression(deref, &derefCompiled, &p)) return false;
        if (p != deref.tokens.size()) return false;
    }

    // Let's store the var identifier for later and parse the array accessor and rhs.
    if (list.tokens[pos].type != Token::token_type::Identifier) return false;
    std::string_view var = list.tokens[pos].value.str;
    pos++;

    // Parse array accessor
    std::vector<CRExpression> dimensions;
    if (_TokenHasValue(list.tokens[pos], SeparatorType::SquareBracketLeft)) {
        pos++;
        while (true) {
            if (_TokenHasValue(list.tokens[pos], SeparatorType::SquareBracketRight)) break;
            CRExpression dim;
            if (!InterpretExpression(list, &dim, &pos)) return false;
            dimensions.push_back(dim);
            if (_TokenHasValue(list.tokens[pos], SeparatorType::Comma)) {
                pos++;
            }
            else if (_TokenHasValue(list.tokens[pos], SeparatorType::SquareBracketRight)) {
                pos++;
                break;
            }
            else
                return false;
        }
    }

    // Figure out set method
    if (list.tokens[pos].type != Token::token_type::Operator) return false;
    CRSetMethod method;
    switch (list.tokens[pos].value.op) {
        case OperatorType::Assign:
            method = CRSetMethod::SM_ASSIGN;
            break;
        case OperatorType::AssignAdd:
            method = CRSetMethod::SM_ADD;
            break;
        case OperatorType::AssignSubtract:
            method = CRSetMethod::SM_SUBTRACT;
            break;
        case OperatorType::AssignMultiply:
            method = CRSetMethod::SM_MULTIPLY;
            break;
        case OperatorType::AssignDivide:
            method = CRSetMethod::SM_DIVIDE;
            break;
        case OperatorType::AssignBinaryAnd:
            method = CRSetMethod::SM_BITWISE_AND;
            break;
        case OperatorType::AssignBinaryOr:
            method = CRSetMethod::SM_BITWISE_OR;
            break;
        case OperatorType::AssignBinaryXor:
            method = CRSetMethod::SM_BITWISE_XOR;
            break;
        default:
            return false;
    }

    // Get RHS
    pos++;
    CRExpression exp;
    if (!InterpretExpression(list, &exp, &pos)) return false;


    // Work out what type of field we're compiling
    unsigned int index;
    switch (_getVarType(var, &index)) {
        case VarType::VARTYPE_FIELD: {
            bool isLocal = _locals.count(index);
            if (dimensions.size()) {
                if (hasDeref)
                    (*output) = new CRActionAssignmentArray(index, method, dimensions, derefCompiled, exp, isLocal);
                else
                    (*output) = new CRActionAssignmentArray(index, method, dimensions, exp, isLocal);
            }
            else {
                if (hasDeref)
                    (*output) = new CRActionAssignmentField(index, method, derefCompiled, exp, isLocal);
                else
                    (*output) = new CRActionAssignmentField(index, method, exp, isLocal);
            }
            break;
        }
        case VarType::VARTYPE_INSTANCE: {
            if (hasDeref)
                (*output) = new CRActionAssignmentInstanceVar(( CRInstanceVar )index, method, dimensions, derefCompiled, exp);
            else
                (*output) = new CRActionAssignmentInstanceVar(( CRInstanceVar )index, method, dimensions, exp);
            break;
        }
        case VarType::VARTYPE_GAME: {
            (*output) = new CRActionAssignmentGameVar(( CRGameVar )index, method, dimensions, exp);
            break;
        }
        default:
            return false;
    }

    _SkipSemicolon(list, pos);
    return true;
}

bool GM8Emulator::Compiler::_InterpretFunction(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    if (list.tokens[pos].type != Token::token_type::Identifier) return false;
    std::string_view functionName = list.tokens[pos].value.str;

    // Parse arguments
    pos++;
    if (!_TokenHasValue(list.tokens[pos], SeparatorType::ParenLeft)) return false;
    pos++;
    std::vector<CRExpression> args;
    while (!_TokenHasValue(list.tokens[pos], SeparatorType::ParenRight)) {
        CRExpression arg;
        if (!InterpretExpression(list, &arg, &pos)) return false;
        args.push_back(arg);
        if (_TokenHasValue(list.tokens[pos], SeparatorType::Comma)) pos++;
    }
    pos++;
    _SkipSemicolon(list, pos);

    // Check if this is a user script (scripts have precedence over internal functions)
    unsigned int scriptCount = AssetManager::GetScriptCount();
    for (unsigned int s = 0; s < scriptCount; s++) {
        Script* scr = AssetManager::GetScript(s);
        if (!scr->exists) continue;
        if (functionName.compare(scr->name) == 0) {
            (*output) = new CRActionRunScript(s, args);
            return true;
        }
    }

    // Check if this is an internal function
    for (unsigned int f = 0; f < _internalFuncNames.size(); f++) {
        if (functionName.compare(_internalFuncNames[f]) == 0) {
            (*output) = new CRActionRunFunction(( CRInternalFunction )f, args);
            return true;
        }
    }

    // Unrecognized function name
    return false;
}

bool GM8Emulator::Compiler::_InterpretBraces(const GM8Emulator::Compiler::TokenList& list, CRAction** output, unsigned int& pos) {
    unsigned int depth = 1;
    TokenList innerList;
    while (++pos) {
        if (_TokenHasValue(list.tokens[pos], SeparatorType::BraceLeft)) depth++;
        if (_TokenHasValue(list.tokens[pos], SeparatorType::BraceRight)) {
            depth--;
            if (!depth) break;
        }
        innerList.tokens.push_back(list.tokens[pos]);
    }
    pos++;

    CRActionList actions;
    if (!Interpret(innerList, &actions)) return false;
    (*output) = new CRActionBlock(actions);
    _SkipSemicolon(list, pos);
    return true;
}


bool GM8Emulator::Compiler::_TokenHasValue(const GM8Emulator::Compiler::Token& token, GM8Emulator::Compiler::KeywordType t) {
    if (token.type == Token::token_type::Keyword) {
        return token.value.key == t;
    }
    return false;
}

bool GM8Emulator::Compiler::_TokenHasValue(const GM8Emulator::Compiler::Token& token, GM8Emulator::Compiler::OperatorType t) {
    if (token.type == Token::token_type::Operator) {
        return token.value.op == t;
    }
    return false;
}

bool GM8Emulator::Compiler::_TokenHasValue(const GM8Emulator::Compiler::Token& token, GM8Emulator::Compiler::SeparatorType t) {
    if (token.type == Token::token_type::Separator) {
        return token.value.sep == t;
    }
    return false;
}

bool GM8Emulator::Compiler::_TokenHasValue(const GM8Emulator::Compiler::Token& token, double t) {
    if (token.type == Token::token_type::Real) {
        return token.value.real == t;
    }
    return false;
}

bool GM8Emulator::Compiler::_TokenHasValue(const GM8Emulator::Compiler::Token& token, std::string t) {
    if (token.type == Token::token_type::String) {
        return !token.value.str.compare(t.c_str());
    }
    return false;
}

void GM8Emulator::Compiler::_SkipSemicolon(const GM8Emulator::Compiler::TokenList& list, unsigned int& pos) {
    if (pos >= list.tokens.size()) return;
    if (_TokenHasValue(list.tokens[pos], SeparatorType::Semicolon)) {
        pos++;
    }
}

bool GM8Emulator::Compiler::_IsAsset(std::string_view& name, unsigned int* index) {
    // These are in order of precedence in the GM8 engine, since two assets can have the same name.
    unsigned int i;
    for (i = 0; i < AssetManager::GetObjectCount(); i++) {
        if (AssetManager::GetObject(i)->exists) {
            if (!name.compare(AssetManager::GetObject(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetSpriteCount(); i++) {
        if (AssetManager::GetSprite(i)->exists) {
            if (!name.compare(AssetManager::GetSprite(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetSoundCount(); i++) {
        if (AssetManager::GetSound(i)->exists) {
            if (!name.compare(AssetManager::GetSound(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetBackgroundCount(); i++) {
        if (AssetManager::GetBackground(i)->exists) {
            if (!name.compare(AssetManager::GetBackground(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetPathCount(); i++) {
        if (AssetManager::GetPath(i)->exists) {
            if (!name.compare(AssetManager::GetPath(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetFontCount(); i++) {
        if (AssetManager::GetFont(i)->exists) {
            if (!name.compare(AssetManager::GetFont(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetTimelineCount(); i++) {
        if (AssetManager::GetTimeline(i)->exists) {
            if (!name.compare(AssetManager::GetTimeline(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetScriptCount(); i++) {
        if (AssetManager::GetScript(i)->exists) {
            if (!name.compare(AssetManager::GetScript(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetRoomCount(); i++) {
        if (AssetManager::GetRoom(i)->exists) {
            if (!name.compare(AssetManager::GetRoom(i)->name)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }
    for (i = 0; i < AssetManager::GetTriggerCount(); i++) {
        if (AssetManager::GetTrigger(i)->exists) {
            if (!name.compare(AssetManager::GetTrigger(i)->constantName)) {
                if (index) (*index) = i;
                return true;
            }
        }
    }

    return false;
}

bool GM8Emulator::Compiler::_IsGMLConst(std::string_view& name, double* value) {
    for (const auto& c : _gmlConsts) {
        if (name.compare(c.first) == 0) {
            if (value) (*value) = static_cast<double>(c.second);
            return true;
        }
    }
    if (name == "pi") {  // LUL
        if (value) (*value) = GML_PI;
        return true;
    }
    return false;
}

bool GM8Emulator::Compiler::_IsGameValue(std::string_view& name, CRGameVar* value) {
    for (unsigned int i = 0; i < _gameValueNames.size(); i++) {
        if (name.compare(_gameValueNames[i]) == 0) {
            if (value) (*value) = static_cast<CRGameVar>(i);
            return true;
        }
    }
    return false;
}

bool GM8Emulator::Compiler::_IsInstanceVar(std::string_view& name, CRInstanceVar* value) {
    for (unsigned int i = 0; i < _instanceVarNames.size(); i++) {
        if (name.compare(_instanceVarNames[i]) == 0) {
            if (value) (*value) = static_cast<CRInstanceVar>(i);
            return true;
        }
    }
    return false;
}

GM8Emulator::Compiler::VarType GM8Emulator::Compiler::_getVarType(std::string_view& name, unsigned int* index) {
    // Game values have highest precedence
    for (unsigned int i = 0; i < _gameValueNames.size(); i++) {
        if (name.compare(_gameValueNames[i]) == 0) {
            if (index) (*index) = i;
            return VARTYPE_GAME;
        }
    }

    // Next, instance variables
    for (unsigned int i = 0; i < _instanceVarNames.size(); i++) {
        if (name.compare(_instanceVarNames[i]) == 0) {
            if (index) (*index) = i;
            return VARTYPE_INSTANCE;
        }
    }

    // If none of the above, it must be a field, let's register it
    unsigned int ix = _RegisterField(name);
    if (index) (*index) = ix;
    return VARTYPE_FIELD;
}