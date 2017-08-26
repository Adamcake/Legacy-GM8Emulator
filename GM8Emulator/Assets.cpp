#include <string.h>
#include <windows.h>
#include "SDL\SDL.h"
#include "Assets.hpp"
#include "CodeAction.hpp"


IndexedEvent::IndexedEvent()
{
	actionCount = 0;
	actions = NULL;
}

IndexedEvent::~IndexedEvent()
{
	delete[] actions;
}


Trigger::Trigger()
{
	name = NULL;
	condition = NULL;
	constantName = NULL;
}

Trigger::~Trigger()
{
	free(name);
	free(condition);
	free(constantName);
}

Constant::Constant()
{
	name = NULL;
	value = NULL;
}

Constant::~Constant()
{
	free(name);
	free(value);
}

Sound::Sound()
{
	name = NULL;
	fileType = NULL;
	fileName = NULL;
	data = NULL;
}

Sound::~Sound()
{
	free(name);
	free(fileType);
	free(fileName);
	free(data);
}

Sprite::Sprite()
{
	name = NULL;
	frames = 0;
	images = NULL;
	collisionMaps = NULL;
}

Sprite::~Sprite()
{
	free(name);

	while (frames) {
		frames--;
		SDL_FreeSurface(images[frames]);
		if (separateCollision || !frames) delete[] collisionMaps[frames].collision;
	}
	free(images);
	delete[] collisionMaps;
}

Background::Background()
{
	name = NULL;
	data = NULL;
}

Background::~Background()
{
	free(name);
	free(data);
}

Path::Path()
{
	name = NULL;
	points = NULL;
}

Path::~Path()
{
	free(name);
	delete[] points;
}

Script::Script()
{
	name = NULL;
	code = NULL;
}

Script::~Script()
{
	free(name);
	free(code);
}

Font::Font()
{
	name = NULL;
	fontName = NULL;
}

Font::~Font()
{
	free(name);
	free(fontName);
}

Timeline::Timeline()
{
	name = NULL;
	momentCount = 0;
	moments = NULL;
}

Timeline::~Timeline()
{
	free(name);
	delete[] moments;
}

Object::Object()
{
	name = NULL;
	evCreateActionCount = 0;
	evCreate = NULL;
	evDestroyActionCount = 0;
	evDestroy = NULL;
	evStepActionCount = 0;
	evStep = NULL;
	evStepBeginActionCount = 0;
	evStepBegin = NULL;
	evStepEndActionCount = 0;
	evStepEnd = NULL;
	evDrawActionCount = 0;
	evDraw = NULL;
}

Object::~Object()
{
	free(name);
	delete[] evCreate;
	delete[] evDestroy;
	delete[] evStep;
	delete[] evStepBegin;
	delete[] evStepEnd;
	delete[] evDraw;

	evAlarm.clear();
	evCollision.clear();
	evKeyboard.clear();
	evKeyPress.clear();
	evKeyRelease.clear();
	evMouse.clear();
	evOther.clear();
	evTrigger.clear();
}

Room::Room()
{
	name = NULL;
	caption = NULL;
	creationCode = NULL;
	backgroundCount = 0;
	backgrounds = NULL;
	viewCount = 0;
	views = NULL;
	instanceCount = 0;
	instances = NULL;
	tileCount = 0;
	tiles = NULL;
}

Room::~Room()
{
	free(name);
	free(caption);
	free(creationCode);

	while (instanceCount) {
		instanceCount--;
		free(instances[instanceCount].creationCode);
	}

	delete[] backgrounds;
	delete[] views;
	delete[] instances;
	delete[] tiles;
}

IncludeFile::IncludeFile()
{
	filename = NULL;
	filepath = NULL;
	exportFolder = NULL;
	data = NULL;

}

IncludeFile::~IncludeFile()
{
	free(filename);
	free(filepath);
	free(exportFolder);
	free(data);
}
