#ifndef _A_ASSETMANAGER_HPP_
#define _A_ASSETMANAGER_HPP_
#include <vector>
#include "Assets.hpp"


void AMClear();

void AMReserveExtensions(unsigned int count);
void AMReserveTriggers(unsigned int count);
void AMReserveConstants(unsigned int count);
void AMReserveSounds(unsigned int count);
void AMReserveSprites(unsigned int count);
void AMReserveBackgrounds(unsigned int count);
void AMReservePaths(unsigned int count);
void AMReserveScripts(unsigned int count);
void AMReserveFonts(unsigned int count);
void AMReserveTimelines(unsigned int count);
void AMReserveObjects(unsigned int count);
void AMReserveRooms(unsigned int count);
void AMReserveIncludeFiles(unsigned int count);

Extension* AMAddExtension();
Trigger* AMAddTrigger();
Constant* AMAddConstant();
Sound* AMAddSound();
Sprite* AMAddSprite();
Background* AMAddBackground();
Path* AMAddPath();
Script* AMAddScript();
Font* AMAddFont();
Timeline* AMAddTimeline();
Object* AMAddObject();
Room* AMAddRoom();
IncludeFile* AMAddIncludeFile();

Extension* AMGetExtension(unsigned int index);
Trigger* AMGetTrigger(unsigned int index);
Constant* AMGetConstant(unsigned int index);
Sound* AMGetSound(unsigned int index);
Sprite* AMGetSprite(unsigned int index);
Background* AMGetBackground(unsigned int index);
Path* AMGetPath(unsigned int index);
Script* AMGetScript(unsigned int index);
Font* AMGetFont(unsigned int index);
Timeline* AMGetTimeline(unsigned int index);
Object* AMGetObject(unsigned int index);
Room* AMGetRoom(unsigned int index);
IncludeFile* AMGetIncludeFile(unsigned int index);

unsigned int AMGetExtensionCount();
unsigned int AMGetTriggerCount();
unsigned int AMGetConstantCount();
unsigned int AMGetSoundCount();
unsigned int AMGetSpriteCount();
unsigned int AMGetBackgroundCount();
unsigned int AMGetPathCount();
unsigned int AMGetScriptCount();
unsigned int AMGetFontCount();
unsigned int AMGetTimelineCount();
unsigned int AMGetObjectCount();
unsigned int AMGetRoomCount();
unsigned int AMGetIncludeFileCount();

#endif