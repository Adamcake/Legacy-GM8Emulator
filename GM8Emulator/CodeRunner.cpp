#include "CodeRunner.hpp"
#include "AssetManager.hpp"
#include "InstanceList.hpp"
#include "CREnums.hpp"


CodeRunner::CodeRunner(AssetManager* assets, InstanceList* instances) {
	_assetManager = assets;
	_instances = instances;
}

CodeRunner::~CodeRunner() {
	for (unsigned int i = 0; i < _codeObjects.size(); i++) {
		free(_codeObjects[i].code);
		free(_codeObjects[i].compiled);
	}
	for (GMLType g : _constants) {
		if (g.state == GML_TYPE_STRING) free(g.sVal);
	}
	for (char* c : _fields) {
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

bool CodeRunner::Run(CodeObject code, Instance* self, Instance* other) {
	char* c = _codeObjects[code].code;
	// tbd
	return true;
}

bool CodeRunner::Query(CodeObject code, Instance * self, Instance * other, bool* response) {
	char* c = _codeObjects[code].code;
	// tbd
	return true;
}


bool CodeRunner::Init() {
	_internalFuncNames.reserve(_INTERNAL_FUNC_COUNT);
	for (unsigned int func = 0; func < _INTERNAL_FUNC_COUNT; func++) {
		switch (func) {
			case EXECUTE_STRING:
				_internalFuncNames.push_back("execute_string");
				break;
			case INSTANCE_CREATE:
				_internalFuncNames.push_back("instance_create");
				break;
			case INSTANCE_DESTROY:
				_internalFuncNames.push_back("instance_destroy");
				break;
			case IRANDOM:
				_internalFuncNames.push_back("irandom");
				break;
			case IRANDOM_RANGE:
				_internalFuncNames.push_back("irandom_range");
				break;
			case MAKE_COLOR_HSV:
				_internalFuncNames.push_back("make_color_hsv");
				break;
			case MOVE_WRAP:
				_internalFuncNames.push_back("move_wrap");
				break;
			case RANDOM:
				_internalFuncNames.push_back("random");
				break;
			case RANDOM_RANGE:
				_internalFuncNames.push_back("random_range");
				break;
			case ROOM_GOTO:
				_internalFuncNames.push_back("room_goto");
				break;
			case ROOM_GOTO_NEXT:
				_internalFuncNames.push_back("room_goto_next");
				break;
			case ROOM_GOTO_PREVIOUS:
				_internalFuncNames.push_back("room_goto_previous");
				break;
			default:
				// There's something in the enum that isn't listed here. Abort
				return false;
		}
	}

	_readOnlyGameValueNames.reserve(_RONLY_GAME_VAR_COUNT);
	for (unsigned int var = 0; var < _RONLY_GAME_VAR_COUNT; var++) {
		switch (var) {
			case ROOM_WIDTH:
				_readOnlyGameValueNames.push_back("room_width");
				break;
			case ROOM_HEIGHT:
				_readOnlyGameValueNames.push_back("room_height");
				break;
			default:
				// There's something in the enum that isn't listed here. Abort
				return false;
		}
	}

	_RWGameValueNames.reserve(_RW_GAME_VAR_COUNT);
	for (unsigned int var = 0; var < _RW_GAME_VAR_COUNT; var++) {
		switch (var) {
			case HEALTH:
				_RWGameValueNames.push_back("health");
				break;
			case LIVES:
				_RWGameValueNames.push_back("lives");
				break;
			case ROOM:
				_RWGameValueNames.push_back("room");
				break;
			case ROOM_SPEED:
				_RWGameValueNames.push_back("room_speed");
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
			default:
				// Something in the enum isn't listed here
				return false;
		}
	}

	_specialVarNames.reserve(5);
	_specialVarNames.push_back("self");
	_specialVarNames.push_back("other");
	_specialVarNames.push_back("all");
	_specialVarNames.push_back("noone");
	_specialVarNames.push_back("global");

	return true;
}

