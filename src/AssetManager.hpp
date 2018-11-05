#ifndef _A_ASSETMANAGER_HPP_
#define _A_ASSETMANAGER_HPP_
#include <vector>
#include "Assets.hpp"
#undef GetObject  // dunno

namespace AssetManager {
    void Clear();

    void ReserveExtensions(unsigned int count);
    void ReserveTriggers(unsigned int count);
    void ReserveConstants(unsigned int count);
    void ReserveSounds(unsigned int count);
    void ReserveSprites(unsigned int count);
    void ReserveBackgrounds(unsigned int count);
    void ReservePaths(unsigned int count);
    void ReserveScripts(unsigned int count);
    void ReserveFonts(unsigned int count);
    void ReserveTimelines(unsigned int count);
    void ReserveObjects(unsigned int count);
    void ReserveRooms(unsigned int count);
    void ReserveIncludeFiles(unsigned int count);

    Extension* AddExtension();
    Trigger* AddTrigger();
    Constant* AddConstant();
    Sound* AddSound();
    Sprite* AddSprite();
    Background* AddBackground();
    Path* AddPath();
    Script* AddScript();
    Font* AddFont();
    Timeline* AddTimeline();
    Object* AddObject();
    Room* AddRoom();
    IncludeFile* AddIncludeFile();

    Extension* GetExtension(unsigned int index);
    Trigger* GetTrigger(unsigned int index);
    Constant* GetConstant(unsigned int index);
    Sound* GetSound(unsigned int index);
    Sprite* GetSprite(unsigned int index);
    Background* GetBackground(unsigned int index);
    Path* GetPath(unsigned int index);
    Script* GetScript(unsigned int index);
    Font* GetFont(unsigned int index);
    Timeline* GetTimeline(unsigned int index);
    Object* GetObject(unsigned int index);
    Room* GetRoom(unsigned int index);
    IncludeFile* GetIncludeFile(unsigned int index);

    unsigned int GetExtensionCount();
    unsigned int GetTriggerCount();
    unsigned int GetConstantCount();
    unsigned int GetSoundCount();
    unsigned int GetSpriteCount();
    unsigned int GetBackgroundCount();
    unsigned int GetPathCount();
    unsigned int GetScriptCount();
    unsigned int GetFontCount();
    unsigned int GetTimelineCount();
    unsigned int GetObjectCount();
    unsigned int GetRoomCount();
    unsigned int GetIncludeFileCount();
}

#endif