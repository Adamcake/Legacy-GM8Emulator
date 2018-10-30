#ifndef _A_CRRUNTIME_HPP_
#define _A_CRRUNTIME_HPP_
#include <pch.h>
struct GMLType;
struct GlobalValues;
struct Instance;
class CRActionList;
class CRExpression;

constexpr int SELF = -1;
constexpr int OTHER = -2;
constexpr int ALL = -3;
constexpr int NOONE = -4;
constexpr int GLOBAL = -5;
constexpr int LOCAL = -7;

namespace Runtime {
    void Init(GlobalValues* globals, std::vector<bool (*)(unsigned int, GMLType*, GMLType*)>& gmlFuncs);
    void Finalize();

    GlobalValues* GetGlobals();
    void SetRoomOrder(unsigned int** order, unsigned int count);

    // Utility functions
    int _round(double);
    bool _equal(double, double);
    bool _isTrue(const GMLType* value);

    // Runtime context
    struct Context {
        Instance* self;
        Instance* other;
        int eventId;
        int eventNumber;
        unsigned int objId;
        unsigned int argc;
        const GMLType* argv;
        std::map<unsigned int, std::map<unsigned int, GMLType>> locals;
        Context() : self(nullptr), other(nullptr), eventId(0), eventNumber(0), objId(0), argc(0), argv(nullptr) {}
        Context(Instance* s, Instance* o, int e, int se, unsigned int oid, unsigned int ac = 0, const GMLType* av = NULL) : self(s), other(o), eventId(e), eventNumber(se), objId(oid), argc(ac), argv(av) {}
        //Context(Instance* o, unsigned int id, unsigned int ac = 0, const GMLType* av = NULL) : other(o), argc(ac), argv(av) {}
    };
    Context GetContext();

    bool Execute(CRActionList&, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, unsigned int argc = 0, GMLType* argv = nullptr);
    bool EvalExpression(CRExpression&, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId, GMLType* out);

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
    bool window_set_caption(unsigned int argc, GMLType* argv, GMLType* out);
    bool window_get_caption(unsigned int argc, GMLType* argv, GMLType* out);
    bool unimplemented(unsigned int argc, GMLType* argv, GMLType* out);
}

#endif