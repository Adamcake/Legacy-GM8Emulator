#include "CodeRunner.hpp"
#include "AssetManager.hpp"
#include "InstanceList.hpp"
#include "CodeActionManager.hpp"
#include "CREnums.hpp"
#include "RNG.hpp"


CodeRunner::CodeRunner(AssetManager* assets, InstanceList* instances, GlobalValues* globals, CodeActionManager* codeActions, GameRenderer* renderer) {
	_renderer = renderer;
	_assetManager = assets;
	_instances = instances;
	_globalValues = globals;
	_codeActions = codeActions;
	memset(_userFiles, 0, sizeof(FILE*));
	RNGRandomize();
}

CodeRunner::~CodeRunner() {
	for (unsigned int i = 0; i < _codeObjects.size(); i++) {
		free(_codeObjects[i].code);
		free(_codeObjects[i].compiled);
	}
	for (char* c : _fieldNames) {
		free(c);
	}
}

CodeObject CodeRunner::Register(char* code, unsigned int len) {
	unsigned int ix = (unsigned int)_codeObjects.size();
	_codeObjects.push_back(CRCodeObject());

	_codeObjects[ix].code = (char*)malloc(len + 1);
	memcpy(_codeObjects[ix].code, code, len);
	_codeObjects[ix].codeLength = len;
	_codeObjects[ix].code[len] = '\0';
	_codeObjects[ix].question = false;
	_codeObjects[ix].compiled = NULL;
	
	return ix;
}

CodeObject CodeRunner::RegisterQuestion(char * code, unsigned int len) {
	unsigned int ix = (unsigned int)_codeObjects.size();
	_codeObjects.push_back(CRCodeObject());

	_codeObjects[ix].code = (char*)malloc(len + 1);
	memcpy(_codeObjects[ix].code, code, len);
	_codeObjects[ix].codeLength = len;
	_codeObjects[ix].code[len] = '\0';
	_codeObjects[ix].question = true;
	_codeObjects[ix].compiled = NULL;

	return ix;
}

bool CodeRunner::Compile(CodeObject object) {
	unsigned char* out;
	if (_codeObjects[object].question) {
		if (!_CompileExpression(_codeObjects[object].code, &out)) return false;
	}
	else {
		if (!_CompileCode(_codeObjects[object].code, &out)) return false;
	}

	_codeObjects[object].compiled = out;
	free(_codeObjects[object].code);
	_codeObjects[object].code = NULL;

	return true;
}



void CodeRunner::SetRoomOrder(unsigned int** order, unsigned int count) {
	_roomOrder = order;
	_roomOrderCount = count;
}

bool CodeRunner::Init() {
	_internalFuncNames.reserve(_INTERNAL_FUNC_COUNT);
	for (unsigned int func = 0; func < _INTERNAL_FUNC_COUNT; func++) {
		switch (func) {
			case COS:
				_internalFuncNames.push_back("cos");
				_gmlFuncs.push_back(&CodeRunner::cos);
				break;
			case EXECUTE_STRING:
				_internalFuncNames.push_back("execute_string");
				_gmlFuncs.push_back(&CodeRunner::execute_string);
				break;
			case INSTANCE_CREATE:
				_internalFuncNames.push_back("instance_create");
				_gmlFuncs.push_back(&CodeRunner::instance_create);
				break;
			case INSTANCE_DESTROY:
				_internalFuncNames.push_back("instance_destroy");
				_gmlFuncs.push_back(&CodeRunner::instance_destroy);
				break;
			case INSTANCE_EXISTS:
				_internalFuncNames.push_back("instance_exists");
				_gmlFuncs.push_back(&CodeRunner::instance_exists);
				break;
			case IRANDOM:
				_internalFuncNames.push_back("irandom");
				_gmlFuncs.push_back(&CodeRunner::irandom);
				break;
			case IRANDOM_RANGE:
				_internalFuncNames.push_back("irandom_range");
				_gmlFuncs.push_back(&CodeRunner::irandom_range);
				break;
			case FILE_BIN_OPEN:
				_internalFuncNames.push_back("file_bin_open");
				_gmlFuncs.push_back(&CodeRunner::file_bin_open);
				break;
			case FILE_BIN_CLOSE:
				_internalFuncNames.push_back("file_bin_close");
				_gmlFuncs.push_back(&CodeRunner::file_bin_close);
				break;
			case FILE_BIN_READ_BYTE:
				_internalFuncNames.push_back("file_bin_read_byte");
				_gmlFuncs.push_back(&CodeRunner::file_bin_read_byte);
				break;
			case FILE_BIN_WRITE_BYTE:
				_internalFuncNames.push_back("file_bin_write_byte");
				_gmlFuncs.push_back(&CodeRunner::file_bin_write_byte);
				break;
			case FLOOR:
				_internalFuncNames.push_back("floor");
				_gmlFuncs.push_back(&CodeRunner::floor);
				break;
			case GAME_RESTART:
				_internalFuncNames.push_back("game_restart");
				_gmlFuncs.push_back(&CodeRunner::game_restart);
				break;
			case KEYBOARD_CHECK:
				_internalFuncNames.push_back("keyboard_check");
				_gmlFuncs.push_back(&CodeRunner::keyboard_check);
				break;
			case KEYBOARD_CHECK_DIRECT:
				_internalFuncNames.push_back("keyboard_check_direct");
				_gmlFuncs.push_back(&CodeRunner::keyboard_check_direct);
				break;
			case KEYBOARD_CHECK_PRESSED:
				_internalFuncNames.push_back("keyboard_check_pressed");
				_gmlFuncs.push_back(&CodeRunner::keyboard_check_pressed);
				break;
			case KEYBOARD_CHECK_RELEASED:
				_internalFuncNames.push_back("keyboard_check_released");
				_gmlFuncs.push_back(&CodeRunner::keyboard_check_released);
				break;
			case MAKE_COLOR_HSV:
				_internalFuncNames.push_back("make_color_hsv");
				_gmlFuncs.push_back(&CodeRunner::make_color_hsv);
				break;
			case MOVE_WRAP:
				_internalFuncNames.push_back("move_wrap");
				_gmlFuncs.push_back(&CodeRunner::move_wrap);
				break;
			case RANDOM:
				_internalFuncNames.push_back("random");
				_gmlFuncs.push_back(&CodeRunner::random);
				break;
			case RANDOM_RANGE:
				_internalFuncNames.push_back("random_range");
				_gmlFuncs.push_back(&CodeRunner::random_range);
				break;
			case RANDOM_GET_SEED:
				_internalFuncNames.push_back("random_get_seed");
				_gmlFuncs.push_back(&CodeRunner::random_get_seed);
				break;
			case RANDOM_SET_SEED:
				_internalFuncNames.push_back("random_set_seed");
				_gmlFuncs.push_back(&CodeRunner::random_set_seed);
				break;
			case ROOM_GOTO:
				_internalFuncNames.push_back("room_goto");
				_gmlFuncs.push_back(&CodeRunner::room_goto);
				break;
			case ROOM_GOTO_NEXT:
				_internalFuncNames.push_back("room_goto_next");
				_gmlFuncs.push_back(&CodeRunner::room_goto_next);
				break;
			case ROOM_GOTO_PREVIOUS:
				_internalFuncNames.push_back("room_goto_previous");
				_gmlFuncs.push_back(&CodeRunner::room_goto_previous);
				break;
			case SIN:
				_internalFuncNames.push_back("sin");
				_gmlFuncs.push_back(&CodeRunner::sin);
				break;
			default:
				// There's something in the enum that isn't listed here. Abort
				return false;
		}
	}

	_gameValueNames.reserve(_GAME_VALUE_COUNT);
	for (unsigned int var = 0; var < _GAME_VALUE_COUNT; var++) {
		switch (var) {
			case HEALTH:
				_gameValueNames.push_back("health");
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

	_operators = { {"^^", OPERATOR_BOOLEAN_XOR}, {"<<", OPERATOR_LSHIFT}, {">>", OPERATOR_RSHIFT}, {"&&", OPERATOR_BOOLEAN_AND}, {"||", OPERATOR_BOOLEAN_OR}, {"==", OPERATOR_EQUALS}, {"!=", OPERATOR_NOT_EQUAL}, {"<=", OPERATOR_LTE}, {">=", OPERATOR_GTE}, {"=", OPERATOR_EQUALS}, {"<", OPERATOR_LT}, {">", OPERATOR_GT}, {"+", OPERATOR_ADD}, {"-", OPERATOR_SUBTRACT}, {"*", OPERATOR_MULTIPLY}, {"/", OPERATOR_DIVIDE}, {"&", OPERATOR_BITWISE_AND}, {"|", OPERATOR_BITWISE_OR}, {"^", OPERATOR_BITWISE_XOR}, {".", OPERATOR_DEREF} };
	_ANOperators = { {"and", OPERATOR_BOOLEAN_AND}, {"or", OPERATOR_BOOLEAN_OR}, {"xor", OPERATOR_BOOLEAN_XOR}, {"mod", OPERATOR_MOD}, {"div", OPERATOR_DIV} };

	_gmlConsts = { { "ANSI_CHARSET", 0 },{ "ARABIC_CHARSET", 178 },{ "BALTIC_CHARSET", 186 },{ "CHINESEBIG5_CHARSET", 136 },{ "DEFAULT_CHARSET", 1 },{ "EASTEUROPE_CHARSET", 238 },{ "GB2312_CHARSET", 134 },{ "GREEK_CHARSET", 161 },{ "HANGEUL_CHARSET", 129 },{ "HEBREW_CHARSET", 177 },{ "JOHAB_CHARSET", 130 },{ "MAC_CHARSET", 77 },{ "OEM_CHARSET", 255 },{ "RUSSIAN_CHARSET", 204 },{ "SHIFTJIS_CHARSET", 128 },{ "SYMBOL_CHARSET", 2 },{ "THAI_CHARSET", 222 },{ "TURKISH_CHARSET", 162 },{ "VIETNAMESE_CHARSET", 163 },{ "all", -3 },{ "bm_add", 1 },{ "bm_dest_alpha", 7 },{ "bm_dest_color", 9 },{ "bm_inv_dest_alpha", 8 },{ "bm_inv_dest_color", 10 },{ "bm_inv_src_alpha", 6 },{ "bm_inv_src_color", 4 },{ "bm_max", 2 },{ "bm_normal", 0 },{ "bm_one", 2 },{ "bm_src_alpha", 5 },{ "bm_src_alpha_sat", 11 },{ "bm_src_color", 3 },{ "bm_subtract", 3 },{ "bm_zero", 1 },{ "button_type", 1 },{ "c_aqua", 16776960 },{ "c_black", 0 },{ "c_blue", 16711680 },{ "c_dkgray", 4210752 },{ "c_fuchsia", 16711935 },{ "c_gray", 8421504 },{ "c_green", 32768 },{ "c_lime", 65280 },{ "c_ltgray", 12632256 },{ "c_maroon", 128 },{ "c_navy", 8388608 },{ "c_olive", 32896 },{ "c_orange", 4235519 },{ "c_purple", 8388736 },{ "c_red", 255 },{ "c_silver", 12632256 },{ "c_teal", 8421376 },{ "c_white", 16777215 },{ "c_yellow", 65535 },{ "cr_appstart", -19 },{ "cr_arrow", -2 },{ "cr_beam", -4 },{ "cr_cross", -3 },{ "cr_default", 0 },{ "cr_drag", -12 },{ "cr_handpoint", -21 },{ "cr_help", -20 },{ "cr_hourglass", -11 },{ "cr_hsplit", -14 },{ "cr_multidrag", -16 },{ "cr_no", -18 },{ "cr_nodrop", -13 },{ "cr_none", -1 },{ "cr_size_all", -22 },{ "cr_size_nesw", -6 },{ "cr_size_ns", -7 },{ "cr_size_nwse", -8 },{ "cr_size_we", -9 },{ "cr_sqlwait", -17 },{ "cr_uparrow", -10 },{ "cr_vsplit", -15 },{ "device_ios_ipad", 2 },{ "device_ios_iphone", 0 },{ "device_ios_iphone_retina", 1 },{ "dll_cdecl", 0 },{ "dll_stdcall", 1 },{ "ef_cloud", 9 },{ "ef_ellipse", 2 },{ "ef_explosion", 0 },{ "ef_firework", 3 },{ "ef_flare", 8 },{ "ef_rain", 10 },{ "ef_ring", 1 },{ "ef_smoke", 4 },{ "ef_smokeup", 5 },{ "ef_snow", 11 },{ "ef_spark", 7 },{ "ef_star", 6 },{ "ev_alarm", 2 },{ "ev_animation_end", 7 },{ "ev_boundary", 1 },{ "ev_close_button", 30 },{ "ev_collision", 4 },{ "ev_create", 0 },{ "ev_destroy", 1 },{ "ev_draw", 8 },{ "ev_end_of_path", 8 },{ "ev_game_end", 3 },{ "ev_game_start", 2 },{ "ev_global_left_button", 50 },{ "ev_global_left_press", 53 },{ "ev_global_left_release", 56 },{ "ev_global_middle_button", 52 },{ "ev_global_middle_press", 55 },{ "ev_global_middle_release", 58 },{ "ev_global_press", 12 },{ "ev_global_release", 13 },{ "ev_global_right_button", 51 },{ "ev_global_right_press", 54 },{ "ev_global_right_release", 57 },{ "ev_joystick1_button1", 21 },{ "ev_joystick1_button2", 22 },{ "ev_joystick1_button3", 23 },{ "ev_joystick1_button4", 24 },{ "ev_joystick1_button5", 25 },{ "ev_joystick1_button6", 26 },{ "ev_joystick1_button7", 27 },{ "ev_joystick1_button8", 28 },{ "ev_joystick1_down", 19 },{ "ev_joystick1_left", 16 },{ "ev_joystick1_right", 17 },{ "ev_joystick1_up", 18 },{ "ev_joystick2_button1", 36 },{ "ev_joystick2_button2", 37 },{ "ev_joystick2_button3", 38 },{ "ev_joystick2_button4", 39 },{ "ev_joystick2_button5", 40 },{ "ev_joystick2_button6", 41 },{ "ev_joystick2_button7", 42 },{ "ev_joystick2_button8", 43 },{ "ev_joystick2_down", 34 },{ "ev_joystick2_left", 31 },{ "ev_joystick2_right", 32 },{ "ev_joystick2_up", 33 },{ "ev_keyboard", 5 },{ "ev_keypress", 9 },{ "ev_keyrelease", 10 },{ "ev_left_button", 0 },{ "ev_left_press", 4 },{ "ev_left_release", 7 },{ "ev_middle_button", 2 },{ "ev_middle_press", 6 },{ "ev_middle_release", 9 },{ "ev_mouse", 6 },{ "ev_mouse_enter", 10 },{ "ev_mouse_leave", 11 },{ "ev_mouse_wheel_down", 61 },{ "ev_mouse_wheel_up", 60 },{ "ev_no_button", 3 },{ "ev_no_more_health", 9 },{ "ev_no_more_lives", 6 },{ "ev_other", 7 },{ "ev_outside", 0 },{ "ev_right_button", 1 },{ "ev_right_press", 5 },{ "ev_right_release", 8 },{ "ev_room_end", 5 },{ "ev_room_start", 4 },{ "ev_step", 3 },{ "ev_step_begin", 1 },{ "ev_step_end", 2 },{ "ev_step_normal", 0 },{ "ev_trigger", 11 },{ "ev_user0", 10 },{ "ev_user1", 11 },{ "ev_user10", 20 },{ "ev_user11", 21 },{ "ev_user12", 22 },{ "ev_user13", 23 },{ "ev_user14", 24 },{ "ev_user15", 25 },{ "ev_user2", 12 },{ "ev_user3", 13 },{ "ev_user4", 14 },{ "ev_user5", 15 },{ "ev_user6", 16 },{ "ev_user7", 17 },{ "ev_user8", 18 },{ "ev_user9", 19 },{ "fa_archive", 32 },{ "fa_bottom", 2 },{ "fa_center", 1 },{ "fa_directory", 16 },{ "fa_hidden", 2 },{ "fa_left", 0 },{ "fa_middle", 1 },{ "fa_readonly", 1 },{ "fa_right", 2 },{ "fa_sysfile", 4 },{ "fa_top", 0 },{ "fa_volumeid", 8 },{ "false", 0 },{ "global", -5 },{ "local", -7 },{ "mb_any", -1 },{ "mb_left", 1 },{ "mb_middle", 3 },{ "mb_none", 0 },{ "mb_right", 2 },{ "noone", -4 },{ "os_android", 5 },{ "os_ios", 4 },{ "os_macosx", 2 },{ "os_psp", 3 },{ "os_win32", 0 },{ "other", -2 },{ "pr_linelist", 2 },{ "pr_linestrip", 3 },{ "pr_pointlist", 1 },{ "pr_trianglefan", 6 },{ "pr_trianglelist", 4 },{ "pr_trianglestrip", 5 },{ "ps_change_all", 0 },{ "ps_change_motion", 2 },{ "ps_change_shape", 1 },{ "ps_deflect_horizontal", 1 },{ "ps_deflect_vertical", 0 },{ "ps_distr_gaussian", 1 },{ "ps_distr_invgaussian", 2 },{ "ps_distr_linear", 0 },{ "ps_force_constant", 0 },{ "ps_force_linear", 1 },{ "ps_force_quadratic", 2 },{ "ps_shape_diamond", 2 },{ "ps_shape_ellipse", 1 },{ "ps_shape_line", 3 },{ "ps_shape_rectangle", 0 },{ "pt_shape_circle", 5 },{ "pt_shape_cloud", 11 },{ "pt_shape_disk", 1 },{ "pt_shape_explosion", 10 },{ "pt_shape_flare", 8 },{ "pt_shape_line", 3 },{ "pt_shape_pixel", 0 },{ "pt_shape_ring", 6 },{ "pt_shape_smoke", 12 },{ "pt_shape_snow", 13 },{ "pt_shape_spark", 9 },{ "pt_shape_sphere", 7 },{ "pt_shape_square", 2 },{ "pt_shape_star", 4 },{ "se_chorus", 1 },{ "se_compressor", 32 },{ "se_echo", 2 },{ "se_equalizer", 64 },{ "se_flanger", 4 },{ "se_gargle", 8 },{ "se_none", 0 },{ "se_reverb", 16 },{ "self", -1 },{ "text_type", 0 },{ "true", 1 },{ "ty_real", 0 },{ "ty_string", 1 },{ "vk_add", 107 },{ "vk_alt", 18 },{ "vk_anykey", 1 },{ "vk_backspace", 8 },{ "vk_control", 17 },{ "vk_decimal", 110 },{ "vk_delete", 46 },{ "vk_divide", 111 },{ "vk_down", 40 },{ "vk_end", 35 },{ "vk_enter", 13 },{ "vk_escape", 27 },{ "vk_f1", 112 },{ "vk_f10", 121 },{ "vk_f11", 122 },{ "vk_f12", 123 },{ "vk_f2", 113 },{ "vk_f3", 114 },{ "vk_f4", 115 },{ "vk_f5", 116 },{ "vk_f6", 117 },{ "vk_f7", 118 },{ "vk_f8", 119 },{ "vk_f9", 120 },{ "vk_home", 36 },{ "vk_insert", 45 },{ "vk_lalt", 164 },{ "vk_lcontrol", 162 },{ "vk_left", 37 },{ "vk_lshift", 160 },{ "vk_multiply", 106 },{ "vk_nokey", 0 },{ "vk_numpad0", 96 },{ "vk_numpad1", 97 },{ "vk_numpad2", 98 },{ "vk_numpad3", 99 },{ "vk_numpad4", 100 },{ "vk_numpad5", 101 },{ "vk_numpad6", 102 },{ "vk_numpad7", 103 },{ "vk_numpad8", 104 },{ "vk_numpad9", 105 },{ "vk_pagedown", 34 },{ "vk_pageup", 33 },{ "vk_pause", 19 },{ "vk_printscreen", 44 },{ "vk_ralt", 165 },{ "vk_rcontrol", 163 },{ "vk_return", 13 },{ "vk_right", 39 },{ "vk_rshift", 161 },{ "vk_shift", 16 },{ "vk_space", 32 },{ "vk_subtract", 109 },{ "vk_tab", 9 },{ "vk_up", 38 } };

	return true;
}

