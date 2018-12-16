#ifndef _A_COLLISION_HPP_
#define _A_COLLISION_HPP_
#include "InstanceList.hpp"

// Should always be called before accessing an instance's bbox variables.
void RefreshInstanceBbox(Instance* i);

// Checks for collision between two instances
bool CollisionCheck(Instance* i1, Instance* i2);

// Checks for an instance collision at a certain point
bool CollisionPointCheck(Instance* i, int x, int y);

// Checks for an instance collision with a given rectangle
bool CollisionRectangleCheck(Instance* i, int x1, int y1, int x2, int y2, bool pixelPerfect);

#endif