#include "AssetManager.hpp"

AssetManager::AssetManager() {
}

AssetManager::~AssetManager() {
	Clear();
}

void AssetManager::Clear() {
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

void AssetManager::ReserveExtensions(unsigned int count) {
	_extensions.reserve(count);
}

void AssetManager::ReserveTriggers(unsigned int count) {
	_triggers.reserve(count);
}

void AssetManager::ReserveConstants(unsigned int count) {
	_constants.reserve(count);
}

void AssetManager::ReserveSounds(unsigned int count) {
	_sounds.reserve(count);
}

void AssetManager::ReserveSprites(unsigned int count) {
	_sprites.reserve(count);
}

void AssetManager::ReserveBackgrounds(unsigned int count) {
	_backgrounds.reserve(count);
}

void AssetManager::ReservePaths(unsigned int count) {
	_paths.reserve(count);
}

void AssetManager::ReserveScripts(unsigned int count) {
	_scripts.reserve(count);
}

void AssetManager::ReserveFonts(unsigned int count) {
	_fonts.reserve(count);
}

void AssetManager::ReserveTimelines(unsigned int count) {
	_timelines.reserve(count);
}

void AssetManager::ReserveObjects(unsigned int count) {
	_objects.reserve(count);
}

void AssetManager::ReserveRooms(unsigned int count) {
	_rooms.reserve(count);
}

void AssetManager::ReserveIncludeFiles(unsigned int count) {
	_includeFiles.reserve(count);
}

Extension * AssetManager::AddExtension() {
	_extensions.push_back(Extension());
	return _extensions._Mylast() - 1;
}

Trigger * AssetManager::AddTrigger() {
	_triggers.push_back(Trigger());
	return _triggers._Mylast() - 1;
}

Constant * AssetManager::AddConstant() {
	_constants.push_back(Constant());
	return _constants._Mylast() - 1;
}

Sound * AssetManager::AddSound() {
	_sounds.push_back(Sound());
	return _sounds._Mylast() - 1;
}

Sprite * AssetManager::AddSprite() {
	_sprites.push_back(Sprite());
	return _sprites._Mylast() - 1;
}

Background * AssetManager::AddBackground() {
	_backgrounds.push_back(Background());
	return _backgrounds._Mylast() - 1;
}

Path * AssetManager::AddPath() {
	_paths.push_back(Path());
	return _paths._Mylast() - 1;
}

Script * AssetManager::AddScript() {
	_scripts.push_back(Script());
	return _scripts._Mylast() - 1;
}

Font * AssetManager::AddFont() {
	_fonts.push_back(Font());
	return _fonts._Mylast() - 1;
}

Timeline * AssetManager::AddTimeline() {
	_timelines.push_back(Timeline());
	return _timelines._Mylast() - 1;
}

Object * AssetManager::AddObject() {
	_objects.push_back(Object());
	return _objects._Mylast() - 1;
}

Room * AssetManager::AddRoom() {
	_rooms.push_back(Room());
	return _rooms._Mylast() - 1;
}

IncludeFile * AssetManager::AddIncludeFile() {
	_includeFiles.push_back(IncludeFile());
	return _includeFiles._Mylast() - 1;
}

Extension * AssetManager::GetExtension(unsigned int index) {
	return _extensions._Myfirst() + index;
}

Trigger * AssetManager::GetTrigger(unsigned int index)
{
	return _triggers._Myfirst() + index;
}

Constant * AssetManager::GetConstant(unsigned int index)
{
	return _constants._Myfirst() + index;
}

Sound * AssetManager::GetSound(unsigned int index)
{
	return _sounds._Myfirst() + index;
}

Sprite * AssetManager::GetSprite(unsigned int index)
{
	return _sprites._Myfirst() + index;
}

Background * AssetManager::GetBackground(unsigned int index)
{
	return _backgrounds._Myfirst() + index;
}

Path * AssetManager::GetPath(unsigned int index)
{
	return _paths._Myfirst() + index;
}

Script * AssetManager::GetScript(unsigned int index)
{
	return _scripts._Myfirst() + index;
}

Font * AssetManager::GetFont(unsigned int index)
{
	return _fonts._Myfirst() + index;
}

Timeline * AssetManager::GetTimeline(unsigned int index)
{
	return _timelines._Myfirst() + index;
}

Object * AssetManager::GetObject(unsigned int index)
{
	return _objects._Myfirst() + index;
}

Room * AssetManager::GetRoom(unsigned int index)
{
	return _rooms._Myfirst() + index;
}

IncludeFile * AssetManager::GetIncludeFile(unsigned int index)
{
	return _includeFiles._Myfirst() + index;
}
