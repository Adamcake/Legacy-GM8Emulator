#include <pch.h>
#include "AssetManager.hpp"

std::vector<Extension> _extensions;
std::vector<Trigger> _triggers;
std::vector<Constant> _constants;
std::vector<Sound> _sounds;
std::vector<Sprite> _sprites;
std::vector<Background> _backgrounds;
std::vector<Path> _paths;
std::vector<Script> _scripts;
std::vector<Font> _fonts;
std::vector<Timeline> _timelines;
std::vector<Object> _objects;
std::vector<Room> _rooms;
std::vector<IncludeFile> _includeFiles;

void AMClear() {
	_extensions.clear();
	_triggers.clear();
	_constants.clear();
	_sounds.clear();
	_sprites.clear();
	_backgrounds.clear();
	_paths.clear();
	_scripts.clear();
	_fonts.clear();
	_timelines.clear();
	_objects.clear();
	_rooms.clear();
}

void AMReserveExtensions(unsigned int count) {
	_extensions.reserve(count);
}

void AMReserveTriggers(unsigned int count) {
	_triggers.reserve(count);
}

void AMReserveConstants(unsigned int count) {
	_constants.reserve(count);
}

void AMReserveSounds(unsigned int count) {
	_sounds.reserve(count);
}

void AMReserveSprites(unsigned int count) {
	_sprites.reserve(count);
}

void AMReserveBackgrounds(unsigned int count) {
	_backgrounds.reserve(count);
}

void AMReservePaths(unsigned int count) {
	_paths.reserve(count);
}

void AMReserveScripts(unsigned int count) {
	_scripts.reserve(count);
}

void AMReserveFonts(unsigned int count) {
	_fonts.reserve(count);
}

void AMReserveTimelines(unsigned int count) {
	_timelines.reserve(count);
}

void AMReserveObjects(unsigned int count) {
	_objects.reserve(count);
}

void AMReserveRooms(unsigned int count) {
	_rooms.reserve(count);
}

void AMReserveIncludeFiles(unsigned int count) {
	_includeFiles.reserve(count);
}

Extension * AMAddExtension() {
	_extensions.push_back(Extension());
	return _extensions.data() + (_extensions.size() - 1);
}

Trigger * AMAddTrigger() {
	_triggers.push_back(Trigger());
	return _triggers.data() + (_triggers.size() - 1);
}

Constant * AMAddConstant() {
	_constants.push_back(Constant());
	return _constants.data() + (_constants.size() - 1);
}

Sound * AMAddSound() {
	_sounds.push_back(Sound());
	return _sounds.data() + (_sounds.size() - 1);
}

Sprite * AMAddSprite() {
	_sprites.push_back(Sprite());
	return _sprites.data() + (_sprites.size() - 1);
}

Background * AMAddBackground() {
	_backgrounds.push_back(Background());
	return _backgrounds.data() + (_backgrounds.size() - 1);
}

Path * AMAddPath() {
	_paths.push_back(Path());
	return _paths.data() + (_paths.size() - 1);
}

Script * AMAddScript() {
	_scripts.push_back(Script());
	return _scripts.data() + (_scripts.size() - 1);
}

Font * AMAddFont() {
	_fonts.push_back(Font());
	return _fonts.data() + (_fonts.size() - 1);
}

Timeline * AMAddTimeline() {
	_timelines.push_back(Timeline());
	return _timelines.data() + (_timelines.size() - 1);
}

Object * AMAddObject() {
	_objects.push_back(Object());
	return _objects.data() + (_objects.size() - 1);
}

Room * AMAddRoom() {
	_rooms.push_back(Room());
	return _rooms.data() + (_rooms.size() - 1);
}

IncludeFile * AMAddIncludeFile() {
	_includeFiles.push_back(IncludeFile());
	return _includeFiles.data() + (_includeFiles.size() - 1);
}

Extension * AMGetExtension(unsigned int index) {
	return _extensions.data() + index;
}

Trigger * AMGetTrigger(unsigned int index)
{
	return _triggers.data() + index;
}

Constant * AMGetConstant(unsigned int index)
{
	return _constants.data() + index;
}

Sound * AMGetSound(unsigned int index)
{
	return _sounds.data() + index;
}

Sprite * AMGetSprite(unsigned int index)
{
	return _sprites.data() + index;
}

Background * AMGetBackground(unsigned int index)
{
	return _backgrounds.data() + index;
}

Path * AMGetPath(unsigned int index)
{
	return _paths.data() + index;
}

Script * AMGetScript(unsigned int index)
{
	return _scripts.data() + index;
}

Font * AMGetFont(unsigned int index)
{
	return _fonts.data() + index;
}

Timeline * AMGetTimeline(unsigned int index)
{
	return _timelines.data() + index;
}

Object * AMGetObject(unsigned int index)
{
	return _objects.data() + index;
}

Room * AMGetRoom(unsigned int index)
{
	return _rooms.data() + index;
}

IncludeFile * AMGetIncludeFile(unsigned int index)
{
	return _includeFiles.data() + index;
}


unsigned int AMGetExtensionCount() { return (unsigned int)_extensions.size(); }
unsigned int AMGetTriggerCount() { return (unsigned int)_triggers.size(); }
unsigned int AMGetConstantCount() { return (unsigned int)_constants.size(); }
unsigned int AMGetSoundCount() { return (unsigned int)_sounds.size(); }
unsigned int AMGetSpriteCount() { return (unsigned int)_sprites.size(); }
unsigned int AMGetBackgroundCount() { return (unsigned int)_backgrounds.size(); }
unsigned int AMGetPathCount() { return (unsigned int)_paths.size(); }
unsigned int AMGetScriptCount() { return (unsigned int)_scripts.size(); }
unsigned int AMGetFontCount() { return (unsigned int)_fonts.size(); }
unsigned int AMGetTimelineCount() { return (unsigned int)_timelines.size(); }
unsigned int AMGetObjectCount() { return (unsigned int)_objects.size(); }
unsigned int AMGetRoomCount() { return (unsigned int)_rooms.size(); }
unsigned int AMGetIncludeFileCount() { return (unsigned int)_includeFiles.size(); }