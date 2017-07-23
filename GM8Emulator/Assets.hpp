#ifndef _GM8_ASSETS_HPP_
#define _GM8_ASSETS_HPP_
#include <vector>
class SDL_Surface;

// Abstract parent type for all 9 assets, just in case it's ever useful.
class Asset {
	private:
		char* name;
	public:
		Asset(char* name);
		virtual ~Asset();

		virtual inline const char* getName() const { return name; }
};


// Subclasses used by main asset types
struct CollisionMap {
	unsigned int top;
	unsigned int bottom;
	unsigned int left;
	unsigned int right;
	unsigned int width;
	unsigned int height;
	bool* collision;
};

// And here are the 11 types of asset found in the gamedata (in order, incidentally.)

class Trigger : public Asset {
	public:
		Trigger(char* name);
		~Trigger();

		char* condition;
		unsigned int checkMoment; // begin step, step, end step
		char* constantName;
};

class Constant : public Asset {
	public:
		Constant(char* name);
		~Constant();

		char* value;
};

class Sound : public Asset {
	public:
		Sound(char* name);
		~Sound();

		unsigned int kind; // normal, background, 3d, use multimedia player
		char* fileType;
		char* fileName;

		unsigned char* data;
		unsigned int dataLength;

		double volume; // Between 1 and 0 (although the lowest it's actually allowed in the editor is 0.3)
		double pan; // Between -1 and 1

		bool preload;
};

class Sprite : public Asset {
	public:
		Sprite(char* name);
		~Sprite();

		unsigned int originX;
		unsigned int originY;

		unsigned int frames;
		SDL_Surface** images;

		bool separateCollision;
};

class Background : public Asset {
	public:
		Background(char* name);
		~Background();
};

class Path : public Asset {
	public:
		Path(char* name);
		~Path();
};

class Script : public Asset {
	public:
		Script(char* name);
		~Script();
};

class Font : public Asset {
	public:
		Font(char* name);
		~Font();
};

class Timeline : public Asset {
	public:
		Timeline(char* name);
		~Timeline();
};

class Object : public Asset {
	public:
		Object(char* name);
		~Object();
};

class Room : public Asset {
	public:
		Room(char* name);
		~Room();
};

#endif