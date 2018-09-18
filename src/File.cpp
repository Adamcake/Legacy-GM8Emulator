#include <pch.h>
#include "CodeRunner.hpp"

bool CodeRunner::file_bin_open(unsigned int argc, GMLType* argv, GMLType* out) {
	//if (argv[0].state != GMLTypeState::String) return false;
	//int ftype = _round(argv[1].dVal);
	//FILE* f;

	//switch (ftype) {
	//	case 0:
	//		if (!fopen_s(&f, argv[0].sVal, "rb")) {
	//			fopen_s(&f, argv[0].sVal, "wb");
	//			fclose(f);
	//			fopen_s(&f, argv[0].sVal, "rb");
	//		}
	//		break;
	//	case 1:
	//		fopen_s(&f, argv[0].sVal, "wb");
	//		break;
	//	default:
	//		fopen_s(&f, argv[0].sVal, "ab"); // Not sure if "ab" is correct for this setting, but I assume so.
	//		break;
	//}

	//int i = 0;
	//for (i = 0; i < 32; i++) {
	//	if (!_userFiles[i]) {
	//		_userFiles[i] = f;
	//	}
	//}

	//if (i == 32) return false;
	//if (out) {
	//	out->state = GMLTypeState::Double;
	//	out->dVal = (double)(i + 1);
	//}

	//return true;
	return false;
}

bool CodeRunner::file_bin_close(unsigned int argc, GMLType* argv, GMLType* out) {
	if (argv[0].state != GMLTypeState::Double) return false;
	int index = _round(argv[0].dVal) - 1;
	if (index < 0 || index >= 32) return false;
	if (!_userFiles[index]) return false;

	fclose(_userFiles[index]);
	_userFiles[index] = NULL;
	//return true;
	return false;
}

bool CodeRunner::file_bin_read_byte(unsigned int argc, GMLType* argv, GMLType* out) {
	/*if (argv[0].state != GMLTypeState::Double) return false;
	int index = _round(argv[0].dVal) - 1;
	if (index < 0 || index >= 32) return false;
	if (!_userFiles[index]) return false;

	unsigned char s;
	fread_s(&s, 1, 1, 1, _userFiles[index]);
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (double)s;
	}
	return true;*/
	return false;
}

bool CodeRunner::file_bin_write_byte(unsigned int argc, GMLType* argv, GMLType* out) {
	/*if (argv[0].state != GMLTypeState::Double || argv[1].state != GMLTypeState::Double) return false;
	int index = _round(argv[0].dVal) - 1;
	if (index < 0 || index >= 32) return false;
	if (!_userFiles[index]) return false;

	unsigned char c = _round(argv[1].dVal);
	fwrite(&c, 1, 1, _userFiles[index]);
	return true;*/
	return false;
}

bool CodeRunner::file_delete(unsigned int argc, GMLType* argv, GMLType* out) {
	/*if (argv[0].state == GMLTypeState::String) {
		remove(argv[0].sVal);
	}
	return true;*/
	return false;
}

bool CodeRunner::file_exists(unsigned int argc, GMLType* argv, GMLType* out) {
	/*if (argv[0].state != GMLTypeState::String) return false;
	if (out) {
		out->state = GMLTypeState::Double;
		out->dVal = (stat(argv[0].sVal, NULL) == 0 ? 1.0 : 0.0);
	}
	return true;*/
	return false;
}