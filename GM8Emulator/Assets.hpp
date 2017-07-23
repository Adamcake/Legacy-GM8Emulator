#ifndef _GM8_ASSETS_HPP_
#define _GM8_ASSETS_HPP_

// Abstract parent type for all 9 assets, just in case it's ever useful.
class Asset {
	private:
		char* name;
	public:
		Asset(char* name);
		virtual ~Asset();

		virtual inline const char* getName() const { return name; }
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

		bool chorus;
		bool echo;
		bool flanger;
		bool reverb;
		bool gargle;

		double volume;
		double pan;

		bool preload;
};

class Sprite : public Asset {
	public:
		Sprite(char* name);
		~Sprite();
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