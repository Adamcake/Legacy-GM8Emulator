#ifndef _A_GLOBALVALUES_HPP_
#define _A_GLOBALVALUES_HPP_
#define ROOM_TO_NONE -1
#define ROOM_TO_NEXT -2
#define ROOM_TO_PREV -3

// These are the GML values that are global (ie. not tied to any one instance.)
struct GlobalValues {
	// Read-write - these can be set either by the Game object or the GML runner
	unsigned int room_speed;
	double health;
	int lives;
	int room_to; // This is what ACTUALLY gets set when you set the "room" variable.

	// Read-only - these can be read by GML but can only be set by the Game object
	unsigned int room;
	unsigned int room_width;
	unsigned int room_height;
};

#endif