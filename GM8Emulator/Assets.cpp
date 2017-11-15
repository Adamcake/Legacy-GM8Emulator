#include <string.h>
#include <windows.h>
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


Extension::Extension()
{
	name = NULL;
	folderName = NULL;
	fileCount = 0;
	files = NULL;
}

Extension::~Extension()
{
	free(name);
	free(folderName);

	while (fileCount) {
		fileCount--;
		free(files[fileCount].filename);
		free(files[fileCount].initializer);
		free(files[fileCount].finalizer);
		free(files[fileCount].data);

		while (files[fileCount].functionCount) {
			files[fileCount].functionCount--;
			free(files[fileCount].functions[files[fileCount].functionCount].name);
			free(files[fileCount].functions[files[fileCount].functionCount].externalName);
		}
		while (files[fileCount].constCount) {
			files[fileCount].constCount--;
			free(files[fileCount].consts[files[fileCount].constCount].name);
			free(files[fileCount].consts[files[fileCount].constCount].value);
		}

		delete[] files[fileCount].functions;
		delete[] files[fileCount].consts;
	}

	delete[] files;
}

Trigger::Trigger()
{
	name = NULL;
	exists = true;
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
	exists = true;
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
	exists = true;
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
	exists = true;
	frameCount = 0;
	frames = NULL;
	collisionMaps = NULL;
}

Sprite::~Sprite()
{
	free(name);

	while (frameCount) {
		frameCount--;
		if (separateCollision || !frameCount) delete[] collisionMaps[frameCount].collision;
	}
	free(frames);
	delete[] collisionMaps;
}

Background::Background()
{
	name = NULL;
	exists = true;
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
	exists = true;
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
	exists = true;
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
	exists = true;
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
	exists = true;
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
	exists = true;
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
	exists = true;
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
	exists = true;
}

IncludeFile::~IncludeFile()
{
	free(filename);
	free(filepath);
	free(exportFolder);
	free(data);
}