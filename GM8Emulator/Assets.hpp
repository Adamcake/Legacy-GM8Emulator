#ifndef _GM8_ASSETS_HPP_
#define _GM8_ASSETS_HPP_
#include <vector>
#include <list>
struct SDL_Surface;
class CodeAction;


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

struct PathPoint {
	double x;
	double y;
	double speed;
};

struct IndexedEvent {
	unsigned int index;
	unsigned int actionCount;
	CodeAction* actions;

	IndexedEvent();
	~IndexedEvent();
};

struct RoomBackground {
	bool visible;
	bool foreground;
	unsigned int backgroundIndex;
	unsigned int x;
	unsigned int y;
	bool tileHor;
	bool tileVert;
	unsigned int hSpeed;
	unsigned int vSpeed;
	unsigned int stretch;
};

struct RoomView {
	bool visible;
	int viewX;
	int viewY;
	unsigned int viewW;
	unsigned int viewH;
	unsigned int portX;
	unsigned int portY;
	unsigned int portW;
	unsigned int portH;
	unsigned int Hbor;
	unsigned int Vbor;
	unsigned int Hsp;
	unsigned int Vsp;
	unsigned int follow;
};

struct RoomInstance {
	int x;
	int y;
	unsigned int objectIndex;
	unsigned int id;
	char* creationCode;
};

struct RoomTile {
	int x;
	int y;
	unsigned int backgroundIndex;
	int tileX;
	int tileY;
	unsigned int width;
	unsigned int height;
	int depth;
	unsigned int id;
};

struct ExtensionFileFunction {
	char* name;
	char* externalName;
	unsigned int convention; // stdcall or cdecl
	unsigned int argCount;
	unsigned int argTypes[17]; // 1=string, 2=real
	unsigned int returnType; // 1=string, 2=real
};

struct ExtensionFileConst {
	char* name;
	char* value;
};

struct ExtensionFile {
	char* filename;
	unsigned int kind; // 1=dll, 2=gml, 3=lib, 4=other
	char* initializer;
	char* finalizer;

	unsigned int functionCount;
	ExtensionFileFunction* functions;

	unsigned int constCount;
	ExtensionFileConst* consts;

	unsigned int dataLength;
	unsigned char* data;
};


// And here are the 13 types of asset found in the gamedata (in order, incidentally.)

class Extension {
	public:
		Extension();
		~Extension();
		char* name;

		char* folderName;
		unsigned int fileCount;
		ExtensionFile* files;
};

class Trigger {
	public:
		Trigger();
		~Trigger();
		char* name;

		char* condition;
		unsigned int checkMoment; // begin step, step, end step
		char* constantName;
};

class Constant {
	public:
		Constant();
		~Constant();
		char* name;

		char* value;
};

class Sound {
	public:
		Sound();
		~Sound();
		char* name;

		unsigned int kind; // normal, background, 3d, use multimedia player
		char* fileType;
		char* fileName;

		unsigned char* data;
		unsigned int dataLength;

		double volume; // Between 1 and 0 (although the lowest it's actually allowed in the editor is 0.3)
		double pan; // Between -1 and 1

		bool preload;
};

class Sprite {
	public:
		Sprite();
		~Sprite();
		char* name;

		unsigned int originX;
		unsigned int originY;

		unsigned int frames;
		SDL_Surface** images;

		bool separateCollision;
		CollisionMap* collisionMaps;
};

class Background {
	public:
		Background();
		~Background();
		char* name;

		unsigned int width;
		unsigned int height;
		unsigned char* data;
};

class Path {
	public:
		Path();
		~Path();
		char* name;

		unsigned int kind;
		bool closed;
		unsigned int precision;
		
		unsigned int pointCount;
		PathPoint* points;
};

class Script {
	public:
		Script();
		~Script();
		char* name;
		char* code;
};

class Font {
	public:
		Font();
		~Font();
		char* name;

		char* fontName;
		unsigned int size;
		bool bold;
		bool italic;
		unsigned int rangeBegin;
		unsigned int rangeEnd;
		
};

class Timeline {
	public:
		Timeline();
		~Timeline();
		char* name;

		unsigned int momentCount;
		IndexedEvent* moments;
};

class Object {
	public:
		Object();
		~Object();
		char* name;

		int spriteIndex;
		bool solid;
		bool visible;
		int depth;
		bool persistent;
		int parentIndex;
		int maskIndex;

		unsigned int evCreateActionCount;
		CodeAction* evCreate;
		unsigned int evDestroyActionCount;
		CodeAction* evDestroy;
		unsigned int evStepActionCount;
		CodeAction* evStep;
		unsigned int evStepBeginActionCount;
		CodeAction* evStepBegin;
		unsigned int evStepEndActionCount;
		CodeAction* evStepEnd;
		unsigned int evDrawActionCount;
		CodeAction* evDraw;

		std::list<IndexedEvent> evAlarm;
		std::list<IndexedEvent> evCollision;
		std::list<IndexedEvent> evKeyboard;
		std::list<IndexedEvent> evKeyPress;
		std::list<IndexedEvent> evKeyRelease;
		std::list<IndexedEvent> evMouse;
		std::list<IndexedEvent> evOther;
		std::list<IndexedEvent> evTrigger;
};

class Room {
	public:
		Room();
		~Room();
		char* name;

		char* caption;
		unsigned int width;
		unsigned int height;
		unsigned int speed;
		bool persistent;
		unsigned int backgroundColour;
		bool drawBackgroundColour;
		char* creationCode;
		bool enableViews;

		unsigned int backgroundCount;
		RoomBackground* backgrounds;
		unsigned int viewCount;
		RoomView* views;
		unsigned int instanceCount;
		RoomInstance* instances;
		unsigned int tileCount;
		RoomTile* tiles;
};

class IncludeFile {
	public:
		IncludeFile();
		~IncludeFile();
		char* filename;
		char* filepath;
		unsigned int dataLength;
		unsigned char* data;
		unsigned int originalSize;
		unsigned int exportFlags;
		char* exportFolder;
		bool overwrite;
		bool freeMemory;
		bool removeAtGameEnd;
};

#endif