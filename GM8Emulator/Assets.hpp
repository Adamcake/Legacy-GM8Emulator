#ifndef _GM8_ASSETS_HPP_
#define _GM8_ASSETS_HPP_

// Abstract parent type for all 9 assets, just in case it's ever useful.
class Asset {
	private:
		char* name;
	public:
		Asset(const char* name);
		virtual ~Asset();

		virtual inline const char* getName() const { return name; }
};


// And here are the 9 types of asset found in the gamedata (in order, incidentally.)

class Sound : public Asset {
	public:
		Sound(const char* name);
		~Sound();
};

class Sprite : public Asset {
	public:
		Sprite(const char* name);
		~Sprite();
};

class Background : public Asset {
	public:
		Background(const char* name);
		~Background();
};

class Path : public Asset {
	public:
		Path(const char* name);
		~Path();
};

class Script : public Asset {
	public:
		Script(const char* name);
		~Script();
};

class Font : public Asset {
	public:
		Font(const char* name);
		~Font();
};

class Timeline : public Asset {
	public:
		Timeline(const char* name);
		~Timeline();
};

class Object : public Asset {
	public:
		Object(const char* name);
		~Object();
};

class Room : public Asset {
	public:
		Room(const char* name);
		~Room();
};

#endif