#pragma once

#include <map>
#include <set>
#include <vector>

typedef unsigned int RImageIndex;
typedef unsigned int CodeObject;
typedef unsigned int CodeAction;

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
    unsigned int actionCount = 0;
    CodeAction* actions = NULL;
};

struct RoomBackground {
    bool visible;
    bool foreground;
    int backgroundIndex;
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
    int follow;
};

struct RoomInstance {
    int x;
    int y;
    unsigned int objectIndex;
    unsigned int id;
    CodeObject creation;
};

struct RoomTile {
    int x;
    int y;
    int backgroundIndex;
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
    unsigned int convention;  // stdcall or cdecl
    unsigned int argCount;
    unsigned int argTypes[17];  // 1=string, 2=real
    unsigned int returnType;    // 1=string, 2=real
};

struct ExtensionFileConst {
    char* name;
    char* value;
};

struct ExtensionFile {
    char* filename;
    unsigned int kind;  // 1=dll, 2=gml, 3=lib, 4=other
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
    bool exists;

    CodeObject codeObj;
    unsigned int checkMoment;  // begin step, step, end step
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
    bool exists;

    unsigned int kind;  // normal, background, 3d, use multimedia player
    char* fileType;
    char* fileName;

    unsigned char* data;
    unsigned int dataLength;

    double volume;  // Between 1 and 0 (although the lowest it's actually allowed in the editor is 0.3)
    double pan;     // Between -1 and 1

    bool preload;
};

class Sprite {
  public:
    Sprite();
    ~Sprite();
    char* name;
    bool exists;

    unsigned int width;
    unsigned int height;

    unsigned int originX;
    unsigned int originY;

    unsigned int frameCount;
    RImageIndex* frames;

    bool separateCollision;
    CollisionMap* collisionMaps;
};

class Background {
  public:
    Background();
    ~Background();
    char* name;
    bool exists;

    unsigned int width;
    unsigned int height;
    // unsigned char* data;
    RImageIndex image;
};

class Path {
  public:
    Path();
    ~Path();
    char* name;
    bool exists;

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
    bool exists;
    CodeObject codeObj;
};

class Font {
  public:
    Font();
    ~Font();
    char* name;
    bool exists;

    char* fontName;
    unsigned int size;
    bool bold;
    bool italic;
    unsigned int charset;
    unsigned int aaLevel;
    unsigned int rangeBegin;
    unsigned int rangeEnd;
    RImageIndex image;
    unsigned int dmap[0x600];
};

class Timeline {
  public:
    Timeline();
    ~Timeline();
    char* name;
    bool exists;

    unsigned int momentCount;
    std::map<unsigned int, IndexedEvent> moments;
};

class Object {
  public:
    Object();
    ~Object();
    char* name;
    bool exists;

    int spriteIndex;
    bool solid;
    bool visible;
    int depth;
    bool persistent;
    int parentIndex;
    int maskIndex;

    std::map<unsigned int, IndexedEvent> events[12];
    std::vector<unsigned int> evList[12];
    std::set<unsigned int> identities;
    std::set<unsigned int> children; // includes extended children
};

class Room {
  public:
    Room();
    ~Room();
    char* name;
    bool exists;

    char* caption;
    unsigned int width;
    unsigned int height;
    unsigned int speed;
    bool persistent;
    unsigned int backgroundColour;
    bool drawBackgroundColour;
    CodeObject creationCode;
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
    bool exists;
};
