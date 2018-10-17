#ifndef _A_CODERUNNER_HPP_
#define _A_CODERUNNER_HPP_

class CodeActionManager;
struct CRStatement;
struct GlobalValues;
struct Instance;
struct CRExpressionElement;
struct GMLType;
enum struct GMLTypeState;
typedef unsigned int CodeObject;
typedef unsigned int InstanceID;
enum CRGameVar;
enum CRInstanceVar;
enum CRVarType;
enum CROperator;
enum CRSetMethod;
#include <pch.h>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <string>
#include "InstanceList.hpp"

#define PI 3.141592654 // Actual value of PI used by the official runner. Please don't make it more accurate.

class CodeRunner {
	private:
		GlobalValues* _globalValues;
		CodeActionManager* _codeActions;

		// Internal code object
		struct CRCodeObject {
			std::string code;
			unsigned int codeLength;
			bool question;
			unsigned char* compiled;
		};
		std::vector<CRCodeObject> _codeObjects;

		// Runtime context
		struct CRContext {
			Instance* self;
			Instance* other;
			int eventId;
			int eventNumber;
			unsigned int objId;
			unsigned int argc;
			GMLType* argv;
			InstanceList::Iterator iterator;
			//std::map<unsigned int, GMLType> locals;
			CRContext(Instance* s, Instance* o, int e, int se, unsigned int oid, unsigned int ac = 0, GMLType* av = NULL) : self(s), other(o), eventId(e), eventNumber(se), objId(oid), argc(ac), argv(av), iterator(0) {}
			CRContext(Instance* o, unsigned int id, unsigned int ac = 0, GMLType* av = NULL) : other(o), iterator(id), argc(ac), argv(av) { self = iterator.Next(); }
		};
		std::stack<CRContext> _contexts;

		// Structure for user file manipulation
		std::fstream _userFiles[32];

		// Field name index during compilation
		std::vector<char*> _fieldNames;

		// Lists of internal names for compiling and running against
		std::vector<const char*> _internalFuncNames;
		std::vector<const char*> _gameValueNames;
		std::vector<const char*> _instanceVarNames;
		std::map<const char*, int> _gmlConsts;
		std::vector<bool(CodeRunner::*)(unsigned int,GMLType*,GMLType*)> _gmlFuncs;

		// Iterator stack for runtime
		std::stack<InstanceList::Iterator> _iterators;

		// The next instance ID to assign when instance_create is called
		unsigned int _nextInstanceID;

		// Room order
		unsigned int** _roomOrder;
		unsigned int _roomOrderCount;

		// Helper functions for running
		int _round(double);
		bool _equal(double, double);
		bool _isTrue(const GMLType* value);
		bool _assertArgs(unsigned int & argc, GMLType *argv, unsigned int arge, bool lenient, ...);

		// "draw" vars
		unsigned int _drawFont = -1;
		int _drawColour = 0;
		int _drawValign = 0;
		int _drawHalign = 0;
		double _drawAlpha = 1.0;

		// GML internal functions
		bool abs(unsigned int argc, GMLType* argv, GMLType* out);
		bool arcsin(unsigned int argc, GMLType* argv, GMLType* out);
		bool arccos(unsigned int argc, GMLType* argv, GMLType* out);
		bool arctan(unsigned int argc, GMLType* argv, GMLType* out);
		bool ceil(unsigned int argc, GMLType* argv, GMLType* out);
		bool choose(unsigned int argc, GMLType* argv, GMLType* out);
		bool cos(unsigned int argc, GMLType* argv, GMLType* out);
		bool degtorad(unsigned int argc, GMLType* argv, GMLType* out);
		bool distance_to_object(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_rectangle(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_alpha(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_color(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_font(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_halign(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_set_valign(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_sprite(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_sprite_ext(unsigned int argc, GMLType* argv, GMLType* out);
		bool draw_text(unsigned int argc, GMLType* argv, GMLType* out);
		bool event_inherited(unsigned int argc, GMLType* argv, GMLType* out);
		bool event_perform(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_open(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_close(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_read_byte(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_bin_write_byte(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_delete(unsigned int argc, GMLType* argv, GMLType* out);
		bool file_exists(unsigned int argc, GMLType* argv, GMLType* out);
		bool floor(unsigned int argc, GMLType* argv, GMLType* out);
		bool game_end(unsigned int argc, GMLType* argv, GMLType* out);
		bool game_restart(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_create(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_destroy(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_exists(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_number(unsigned int argc, GMLType* argv, GMLType* out);
		bool instance_position(unsigned int argc, GMLType* argv, GMLType* out);
		bool irandom(unsigned int argc, GMLType* argv, GMLType* out);
		bool irandom_range(unsigned int argc, GMLType* argv, GMLType* out);
		bool is_real(unsigned int argc, GMLType* argv, GMLType* out);
		bool is_string(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check_direct(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check_pressed(unsigned int argc, GMLType* argv, GMLType* out);
		bool keyboard_check_released(unsigned int argc, GMLType* argv, GMLType* out);
		bool ln(unsigned int argc, GMLType* argv, GMLType* out);
		bool log2(unsigned int argc, GMLType* argv, GMLType* out);
		bool log10(unsigned int argc, GMLType* argv, GMLType* out);
		bool logn(unsigned int argc, GMLType* argv, GMLType* out);
		bool make_color_hsv(unsigned int argc, GMLType* argv, GMLType* out);
		bool max(unsigned int argc, GMLType* argv, GMLType* out);
		bool min(unsigned int argc, GMLType* argv, GMLType* out);
        bool motion_set(unsigned int argc, GMLType* argv, GMLType* out);
        bool move_bounce_solid(unsigned int argc, GMLType* argv, GMLType* out);
		bool move_contact_solid(unsigned int argc, GMLType* argv, GMLType* out);
		bool move_wrap(unsigned int argc, GMLType* argv, GMLType* out);
		bool ord(unsigned int argc, GMLType* argv, GMLType* out);
		bool place_free(unsigned int argc, GMLType* argv, GMLType* out);
		bool place_meeting(unsigned int argc, GMLType* argv, GMLType* out);
		bool point_direction(unsigned int argc, GMLType* argv, GMLType* out);
        bool point_distance(unsigned int argc, GMLType* argv, GMLType* out);
		bool power(unsigned int argc, GMLType* argv, GMLType* out);
		bool random(unsigned int argc, GMLType* argv, GMLType* out);
		bool random_range(unsigned int argc, GMLType* argv, GMLType* out);
		bool random_get_seed(unsigned int argc, GMLType* argv, GMLType* out);
		bool random_set_seed(unsigned int argc, GMLType* argv, GMLType* out);
		bool radtodeg(unsigned int argc, GMLType* argv, GMLType* out);
		bool room_goto(unsigned int argc, GMLType* argv, GMLType* out);
		bool room_goto_next(unsigned int argc, GMLType* argv, GMLType* out);
		bool room_goto_previous(unsigned int argc, GMLType* argv, GMLType* out);
		bool round(unsigned int argc, GMLType* argv, GMLType* out);
		bool sign(unsigned int argc, GMLType* argv, GMLType* out);
		bool sin(unsigned int argc, GMLType* argv, GMLType* out);
		bool sqr(unsigned int argc, GMLType* argv, GMLType* out);
		bool sqrt(unsigned int argc, GMLType* argv, GMLType* out);
		bool string(unsigned int argc, GMLType* argv, GMLType* out);
		bool string_width(unsigned int argc, GMLType* argv, GMLType* out);
		bool string_height(unsigned int argc, GMLType* argv, GMLType* out);
		bool tan(unsigned int argc, GMLType* argv, GMLType* out);
		bool window_set_caption(unsigned int argc, GMLType *argv, GMLType *out);
		bool window_get_caption(unsigned int argc, GMLType *argv, GMLType *out);
		bool unimplemented(unsigned int argc, GMLType* argv, GMLType* out);

	public:
		CodeRunner(GlobalValues* globals, CodeActionManager* codeActions);
		~CodeRunner();

		// For populating the constant lists. This should be called before anything else.
		bool Init();

		// Set a specific room order. Usually done after loading the room order from the exe.
		void SetRoomOrder(unsigned int** order, unsigned int count);

		// Set the next instance ID to assign after all the static instances are loaded
		inline void SetNextInstanceID(unsigned int i) { _nextInstanceID = i; }

		// Register a code block to be compiled. Returns a unique reference to that code action to be later Compile()d and Run().
		CodeObject Register(char* code, unsigned int length);

		// Register a code expression to be compiled. Returns a unique reference to that code action to be later Compile()d and Query()d.
		CodeObject RegisterQuestion(char* code, unsigned int length);

		// Compile a code object that has been returned by Register(). Returns true on success, false on error (ie. game should close.)
		// Be sure to call this only after the AssetManager is fully loaded.
		bool Compile(CodeObject object);

		// Run a compiled code object. Returns true on success, false on error (ie. the game should close.)
		// Most be passed the instance ID of the "self" and "other" instances in this context. (both may be NULL)
		// ev and sub indicate the event that's being run. For more info, check the "COMPILED OBJECT EVENTS" section of notes.txt
		bool Run(CodeObject code, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, unsigned int argc = 0, GMLType* argv = NULL);

		// Run a compiled GML question (boolean expression). Returns true on success, false on error (ie. the game should close.)
		// The output value is stored in the supplied pointer.
		bool Query(CodeObject code, Instance* self, Instance* other, bool* response);
};

#endif