#pragma once
#include "CRGMLType.hpp"

typedef unsigned int InstanceID;

// I've tried to name all the instance variables how they're referred to in GML where applicable.
// read-only properties not included: sprite_width/height, sprite_xoffset/yoffset, image_number, bbox_bottom/left/right/top
struct Instance {
    bool exists;
    InstanceID id;
    int object_index;
    bool solid;
    bool visible;
    bool persistent;
    int depth;
    int sprite_index;
    double image_alpha;
    int image_blend;
    double image_index;
    double image_speed;
    double image_xscale;
    double image_yscale;
    double image_angle;
    int mask_index;
    double direction;
    double friction;
    double gravity;
    double gravity_direction;
    double hspeed;
    double vspeed;
    double speed;
    double x;
    double y;
    double xprevious;
    double yprevious;
    double xstart;
    double ystart;
    int path_index;
    double path_position;  // Normalized from 0 to 1
    double path_positionprevious;
    double path_speed;
    double path_scale;
    double path_orientation;
    int path_endaction;  // 0 = stop, 1 = continue from start, 2 = continue from here, 4 = reverse // https://docs.yoyogames.com/source/dadiospice/002_reference/paths/path_start.html
    int timeline_index;
    bool timeline_running;
    double timeline_speed;
    double timeline_position;
    bool timeline_loop;

    int bbox_top;
    int bbox_left;
    int bbox_right;
    int bbox_bottom;
    bool bboxIsStale;

    std::map<unsigned int, std::map<int, GMLType>> _fields;
    std::map<unsigned int, int> _alarms;
};
