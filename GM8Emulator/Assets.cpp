#include <string.h>
#include <windows.h>
#include "SDL\SDL.h"
#include "Assets.hpp"

Asset::Asset(char* pName) {
	name = pName;
}

Asset::~Asset() {
	free(name);
}



Trigger::Trigger(char * pName) : Asset(pName)
{
	condition = NULL;
	constantName = NULL;
}

Trigger::~Trigger()
{
	free(condition);
	free(constantName);
}

Constant::Constant(char * pName) : Asset(pName)
{
	value = NULL;
}

Constant::~Constant()
{
	free(value);
}

Sound::Sound(char * pName) : Asset(pName)
{
	fileType = NULL;
	fileName = NULL;
	data = NULL;
}

Sound::~Sound()
{
	free(fileType);
	free(fileName);
	free(data);
}

Sprite::Sprite(char * pName) : Asset(pName)
{
	images = NULL;
}

Sprite::~Sprite()
{
	if (separateCollision) {
		for (unsigned int i = 0; i < frames; i++) {
			delete[]((CollisionMap*)(images[i]->userdata))->collision;
			delete[] images[i]->userdata;
			SDL_FreeSurface(images[i]);
		}
	}
	else {
		delete[]((CollisionMap*)(images[0]->userdata))->collision;
		delete[] images[0]->userdata;
		for (unsigned int i = 0; i < frames; i++) {
			SDL_FreeSurface(images[i]);
		}
	}

	delete[] images;
}

Background::Background(char * pName) : Asset(pName)
{
}

Background::~Background()
{
}

Path::Path(char * pName) : Asset(pName)
{
}

Path::~Path()
{
}

Script::Script(char * pName) : Asset(pName)
{
}

Script::~Script()
{
}

Font::Font(char * pName) : Asset(pName)
{
}

Font::~Font()
{
}

Timeline::Timeline(char * pName) : Asset(pName)
{
}

Timeline::~Timeline()
{
}

Object::Object(char * pName) : Asset(pName)
{
}

Object::~Object()
{
}

Room::Room(char * pName) : Asset(pName)
{
}

Room::~Room()
{
}
