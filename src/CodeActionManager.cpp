#include <stdlib.h>
#include "CodeActionManager.hpp"
#include "CodeRunner.hpp"
#include "Instance.hpp"
#include "StreamUtil.hpp"


bool CodeActionManager::Read(const unsigned char* stream, unsigned int* pos, CodeAction* out) {
	CACodeAction action;

	(*pos) += 8; // Skips version id and useless lib id
	action.actionID = ReadDword(stream, pos);
	(*pos) += 8; // Skips the useless "kind" variable and a flag for whether it can be relative
	action.question = ReadDword(stream, pos);
	(*pos) += 8; // Skips a flag for whether this action applies to something, and the useless "type" var
	(*pos) += ReadDword(stream, pos); // Function name?
	(*pos) += ReadDword(stream, pos); // Function code?

	unsigned int argCount = ReadDword(stream, pos);
	if (argCount > 8) return false; // There's space for 8 args per action. If we try to read more than this, something's wrong.

	(*pos) += 36; // Version id and unused arg type values

	int appliesToObject = ReadDword(stream, pos);
	bool relative = ReadDword(stream, pos);

	(*pos) += 4; // Another version id..

	char** args = new char*[argCount];
	unsigned int lengths[8];
	unsigned int i;
	for (i = 0; i < argCount; i++) {
		args[i] = ReadString(stream, pos, lengths+i);
	}

	(*pos) += ((8 - i) * 5); // Skip unused arg strings. These should all be 1-length strings that say "0".
	bool not = ReadDword(stream, pos);

	// Now we have to generate some GML and register this with the code runner.
	char* gml = NULL;
	unsigned int gmlLen = 0;
	switch (action.actionID) {
		case 101: {
			// Start moving in a direction
			const char* baseGml = "direction=%d;speed=%s;";
			if (relative) baseGml = "direction=%d;speed+=%s;";
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
			if(dirCount) direction /= dirCount;
			gmlLen = (unsigned int)strlen(baseGml) + 20 + lengths[1] - (2 * 2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, direction, args[1]);
			break;
		}
		case 102: {
			// Set direction and speed
			const char* baseGml = "direction=%s;speed=%s;";
			if(relative) baseGml = "direction=%s;speed+=%s;";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] - (2*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1]);
			break;
		}
		case 105: {
			// Move towards point
			const char* baseGml = "move_towards_point(%s,%s,%s)";
			if(relative) baseGml = "move_towards_point(%s+x,%s+y,%s)";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] + lengths[2] - (3*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1], args[2]);
			break;
		}
		case 107: {
			// Set gravity
			const char* baseGml = "gravity=%s;gravity_direction=%s;";
			if (relative) baseGml = "gravity+=%s;gravity_direction=%s;";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] - (2*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[1], args[0]);
			break;
		}
		case 109: {
			// Jump to position
			const char* baseGml = "x=%s;y=%s;";
			if (relative) baseGml = "x+=%s;y+=%s;";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] - (2 * 2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[1], args[0]);
			break;
		}
		case 112: {
			// Wrap around screen - Note: I think this is right. It's been accurate in every case I've tested so far.
			const char* baseGml = "%s%s";
			const char* baseHorGml = "if((hspeed>0)&&(x>=room_width))x-=(room_width+sprite_width)if((hspeed<0)&&(x<0))x+=(room_width+sprite_width)";
			const char* baseVertGml = "if((vspeed>0)&&(y>=room_height))y-=(room_height+sprite_height)if((vspeed<0)&&(y<0))y+=(room_height+sprite_height)";
			bool hor = true;
			bool vert = true;
			if (args[0][0] == '1') hor = false;
			else if (args[0][0] == '0') vert = false;
			gmlLen = (unsigned int)((hor ? strlen(baseHorGml) : 0) + (vert ? strlen(baseVertGml) : 0));
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, (hor? baseHorGml : ""), (vert? baseVertGml : ""));
			break;
		}
		case 115: {
			// Move bounce
			const char* baseGml = "%s(%s)";
			const char* funcName = (args[1][0] == '0') ? "move_bounce_solid" : "move_bounce_all";
			gmlLen = (unsigned int)(strlen(baseGml) + strlen(funcName) + lengths[1] - (2 * 2));
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, funcName, args[0]);
			break;
		}
		case 119: {
			// Set path
			const char* baseGml = "path_start(%s,%s,%s,%s)";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] + lengths[2] + lengths[3] - (4*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1], args[2], args[3]);
			break;
		}
		case 201: {
			// Create an instance
			const char* baseGml = "instance_create(%s,%s,%s)";
			if(relative) baseGml = "instance_create(%s+x,%s+y,%s)";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] + lengths[2] - (3*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[1], args[2], args[0]);
			break;
		}
		case 202: {
			// Change the instance
			const char* baseGml = "instance_change(%s,%s)";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] - (2 * 2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1]);
			break;
		}
		case 203: {
			// Destroy the instance
			const char* baseGml = "instance_destroy()";
			gmlLen = (unsigned int)strlen(baseGml);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml);
			break;
		}
		case 204: {
			// Destroy instances at position
			const char* baseGml = "position_destroy(%s,%s)";
			if(relative) baseGml = "position_destroy(%s+x,%s+y)";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] - (2*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1]);
			break;
		}
		case 206: {
			// Create a moving instance
			const char* baseGml = "var a; a = instance_create(%s,%s,%s);a.speed=%s;a.direction=%s;";
			if(relative) baseGml = "var a; a = instance_create(%s+x,%s+y,%s);a.speed=%s;a.direction=%s;";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] + lengths[2] + lengths[3] + lengths[4] - (5*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[1], args[2], args[0], args[3], args[4]);
			break;
		}
		case 222: {
			// Go to next room
			const char* baseGml = "transition_kind=%s;room_goto_next()";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] - 2;
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0]);
			break;
		}
		case 301: {
			// Set alarm
			const char* baseGml = "alarm[%s]=%s";
			if(relative) baseGml = "alarm[%s]+=%s";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] - (2 * 2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[1], args[0]);
			break;
		}
		case 305: {
			// Set timeline
			const char* baseGml = "timeline_index=%s;timeline_position=%s;timeline_running=%s;timeline_loop=%s;";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] + lengths[2] + lengths[3] - (4*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1], args[2], args[3]);
			break;
		}
		case 308: {
			// Stop and reset timeline
			const char* baseGml = "timeline_running=0;timeline_position=0;";
			gmlLen = (unsigned int)strlen(baseGml);
			gml = (char*)malloc(gmlLen + 1);
			memcpy(gml, baseGml, gmlLen);
			gml[gmlLen] = '\0';
			break;
		}
		case 309: {
			// Set timeline speed
			const char* baseGml = "timeline_speed=%s";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] - 2;
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0]);
			break;
		}
		case 404: {
			// Test the number of instances
			const char* baseGml = "instance_number(%s) %s (%s)";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + 2 + lengths[1] - (3*2);
			gml = (char*)malloc(gmlLen + 1);

			char* op = "";
			switch (args[2][0]) {
				case '0':
					op = not? "!=" : "==";
					break;
				case '1':
					op = not? ">=" : "<";
					break;
				case '2':
					op = not? "<=" : ">";
					break;
			}
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], op, args[1]);
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
		case 532: {
			// Create an effect
			const char* baseGml = "%s(%s,%s,%s,%s,%s)";
			if(relative) baseGml = "%s(%s,%s+x,%s+y,%s,%s)";
			const char* funcName = "effect_create_above";
			if (args[5][0] == '0') funcName = "effect_create_below";
			gmlLen = (unsigned int)(strlen(baseGml) + strlen(funcName) + lengths[0] + lengths[1] + lengths[2] + lengths[3] + lengths[4] - (6 * 2));
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, funcName, args[0], args[1], args[2], args[3], args[4]);
			break;
		}
		case 541: {
			// Change sprite
			const char* baseGml = "sprite_index=%s;image_index=%s;image_speed=%s;";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] + lengths[2] - (3 * 2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1], args[2]);
			break;
		}
		case 542: {
			// Transform the sprite
			const char* baseGml = "image_xscale=%s%s;image_yscale=%s%s;image_angle=%s;";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] + lengths[2] + 2 - (3 * 2);
			gml = (char*)malloc(gmlLen + 1);
			bool xflip = args[3][0] == '1' || args[3][0] == '3';
			bool yflip = args[3][0] == '2' || args[3][0] == '3';
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, (xflip? "-" : ""), args[0], (yflip? "-" : ""), args[1], args[2]);
			break;
		}
		case 603: {
			// Execute a block of code
			gmlLen = lengths[0];
			gml = (char*)malloc(gmlLen + 1);
			memcpy(gml, args[0], gmlLen);
			gml[gmlLen] = '\0';
			break;
		}
		case 605: {
			// Comment
			break;
		}
		case 611: {
			// Set variable
			const char* baseGml = "%s=%s";
			if (relative) baseGml = "%s+=%s";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] + lengths[1] - (2*2);
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0], args[1]);
			break;
		}
		case 612: {
			// Test variable
			const char* baseGml = "%s%s%s";
			gmlLen = (unsigned int)lengths[0] + 2 + lengths[1];
			gml = (char*)malloc(gmlLen + 1);

			char* op = "";
			switch (args[2][0]) {
			case '0':
				op = not? "!=" : "==";
				break;
			case '1':
				op = not? ">=" : "<";
				break;
			case '2':
				op = not? "<=" : ">";
				break;
			}
			sprintf_s(gml, gmlLen + 1, baseGml, args[0], op, args[1]);
			break;
		}
		case 721: {
			// Set health
			const char* baseGml = "health=%s";
			if (relative) baseGml = "health+=%s";
			gmlLen = (unsigned int)strlen(baseGml) + lengths[0] - 2;
			gml = (char*)malloc(gmlLen + 1);
			gmlLen = (unsigned int)sprintf_s(gml, gmlLen + 1, baseGml, args[0]);
			break;
		}
		default: {
			// Unknown
			break;
		}
	}

	// Add a "with" statement for the thing this action applies to
	if (appliesToObject != -1) { // -1 is "self". We could omit this check and add a with(self) clause if we wanted to, but it'd be pointless and slow down the application.
		const char* finalGmlFormat = "with(%i){%s}";
		char* finalGml = (char*)malloc(gmlLen + 20);
		unsigned int finalLen = sprintf_s(finalGml, gmlLen + 20, finalGmlFormat, appliesToObject, gml);
		free(gml);
		gml = finalGml;
		gmlLen = finalLen;
	}

	// Register the code we just generated
	if (action.question) {
		action.codeObj = _runner->RegisterQuestion(gml, gmlLen);
	}
	else {
		action.codeObj = _runner->Register(gml, gmlLen);
	}

	// Clean up
	(*out) = (CodeAction)_actions.size();
	_actions.push_back(action);
	free(gml);
	for (i = 0; i < argCount; i++) {
		free(args[i]);
	}
	delete args;
	return true;
}

bool CodeActionManager::Compile(CodeAction action) {
	return _runner->Compile(_actions[action].codeObj);
}

bool CodeActionManager::Run(CodeAction* actions, unsigned int count, Instance* self, Instance* other) {
	unsigned int pos = 0;
	while (pos < count) {
		bool run = true;
		if (_actions[actions[pos]].question) {
			// Multiple questions can be chained together with the end statement dependent on all of them.
			while (_actions[actions[pos]].question) {
				if (run) {
					bool r;
					if (!_runner->Query(_actions[actions[pos]].codeObj, self, other, &r)) return false;
					run &= r;
				}
				pos++;
			}
		}

		if(run) {
			if (!_runner->Run(_actions[actions[pos]].codeObj, self, other)) return false;
			pos++;
		}
		else {
			// Not a question and we are NOT going to run it.
			if(_actions[actions[pos]].actionID == 422) {
				// "start block", so skip until matching end block
				pos++;
				unsigned int depth = 1;
				while (depth) {
					if (_actions[actions[pos]].actionID == 422) depth++;
					else if (_actions[actions[pos]].actionID == 424) depth--;
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