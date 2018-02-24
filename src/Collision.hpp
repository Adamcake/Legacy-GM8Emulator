#ifndef _A_COLLISION_HPP_
#define _A_COLLISION_HPP_
struct Instance;
class AssetManager;

// Should always be called before accessing an instance's bbox variables.
void RefreshInstanceBbox(Instance* i, AssetManager* assets);

// Checks for collision between two instances
bool CollisionCheck(Instance* i1, Instance* i2, AssetManager* assets);

#endif