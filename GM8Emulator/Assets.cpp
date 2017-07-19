#include <string.h>
#include "Assets.hpp"

Asset::Asset(const char* pName) {
	unsigned int len = strlen(name);
	this->name = new char[len + 1];
	memcpy(this->name, name, len + 1);
}

Asset::~Asset() {
	delete[] name;
}



Sound::Sound(const char * pName) : Asset(pName)
{
}

Sound::~Sound()
{
}

Sprite::Sprite(const char * pName) : Asset(pName)
{
}

Sprite::~Sprite()
{
}

Background::Background(const char * pName) : Asset(pName)
{
}

Background::~Background()
{
}

Path::Path(const char * pName) : Asset(pName)
{
}

Path::~Path()
{
}

Script::Script(const char * pName) : Asset(pName)
{
}

Script::~Script()
{
}

Font::Font(const char * pName) : Asset(pName)
{
}

Font::~Font()
{
}

Timeline::Timeline(const char * pName) : Asset(pName)
{
}

Timeline::~Timeline()
{
}

Object::Object(const char * pName) : Asset(pName)
{
}

Object::~Object()
{
}

Room::Room(const char * pName) : Asset(pName)
{
}

Room::~Room()
{
}
