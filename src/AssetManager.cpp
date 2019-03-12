#include <algorithm>
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
std::map<unsigned int, std::vector<unsigned int>> _eventHolderList[12];

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

void AssetManager::ReserveExtensions(unsigned int count) { _extensions.reserve(count); }
void AssetManager::ReserveTriggers(unsigned int count) { _triggers.reserve(count); }
void AssetManager::ReserveConstants(unsigned int count) { _constants.reserve(count); }
void AssetManager::ReserveSounds(unsigned int count) { _sounds.reserve(count); }
void AssetManager::ReserveSprites(unsigned int count) { _sprites.reserve(count); }
void AssetManager::ReserveBackgrounds(unsigned int count) { _backgrounds.reserve(count); }
void AssetManager::ReservePaths(unsigned int count) { _paths.reserve(count); }
void AssetManager::ReserveScripts(unsigned int count) { _scripts.reserve(count); }
void AssetManager::ReserveFonts(unsigned int count) { _fonts.reserve(count); }
void AssetManager::ReserveTimelines(unsigned int count) { _timelines.reserve(count); }
void AssetManager::ReserveObjects(unsigned int count) { _objects.reserve(count); }
void AssetManager::ReserveRooms(unsigned int count) { _rooms.reserve(count); }
void AssetManager::ReserveIncludeFiles(unsigned int count) { _includeFiles.reserve(count); }

Extension* AssetManager::AddExtension() {
    _extensions.push_back(Extension());
    return _extensions.data() + (_extensions.size() - 1);
}

Trigger* AssetManager::AddTrigger() {
    _triggers.push_back(Trigger());
    return _triggers.data() + (_triggers.size() - 1);
}

Constant* AssetManager::AddConstant() {
    _constants.push_back(Constant());
    return _constants.data() + (_constants.size() - 1);
}

Sound* AssetManager::AddSound() {
    _sounds.push_back(Sound());
    return _sounds.data() + (_sounds.size() - 1);
}

Sprite* AssetManager::AddSprite() {
    _sprites.push_back(Sprite());
    return _sprites.data() + (_sprites.size() - 1);
}

Background* AssetManager::AddBackground() {
    _backgrounds.push_back(Background());
    return _backgrounds.data() + (_backgrounds.size() - 1);
}

Path* AssetManager::AddPath() {
    _paths.push_back(Path());
    return _paths.data() + (_paths.size() - 1);
}

Script* AssetManager::AddScript() {
    _scripts.push_back(Script());
    return _scripts.data() + (_scripts.size() - 1);
}

Font* AssetManager::AddFont() {
    _fonts.push_back(Font());
    return _fonts.data() + (_fonts.size() - 1);
}

Timeline* AssetManager::AddTimeline() {
    _timelines.push_back(Timeline());
    return _timelines.data() + (_timelines.size() - 1);
}

Object* AssetManager::AddObject() {
    _objects.push_back(Object());
    return _objects.data() + (_objects.size() - 1);
}

Room* AssetManager::AddRoom() {
    _rooms.push_back(Room());
    return _rooms.data() + (_rooms.size() - 1);
}

IncludeFile* AssetManager::AddIncludeFile() {
    _includeFiles.push_back(IncludeFile());
    return _includeFiles.data() + (_includeFiles.size() - 1);
}

Extension* AssetManager::GetExtension(unsigned int index) { return _extensions.data() + index; }
Trigger* AssetManager::GetTrigger(unsigned int index) { return _triggers.data() + index; }
Constant* AssetManager::GetConstant(unsigned int index) { return _constants.data() + index; }
Sound* AssetManager::GetSound(unsigned int index) { return _sounds.data() + index; }
Sprite* AssetManager::GetSprite(unsigned int index) { return _sprites.data() + index; }
Background* AssetManager::GetBackground(unsigned int index) { return _backgrounds.data() + index; }
Path* AssetManager::GetPath(unsigned int index) { return _paths.data() + index; }
Script* AssetManager::GetScript(unsigned int index) { return _scripts.data() + index; }
Font* AssetManager::GetFont(unsigned int index) { return _fonts.data() + index; }
Timeline* AssetManager::GetTimeline(unsigned int index) { return _timelines.data() + index; }
Object* AssetManager::GetObject(unsigned int index) { return _objects.data() + index; }
Room* AssetManager::GetRoom(unsigned int index) { return _rooms.data() + index; }
IncludeFile* AssetManager::GetIncludeFile(unsigned int index) { return _includeFiles.data() + index; }


unsigned int AssetManager::GetExtensionCount() { return static_cast<unsigned int>(_extensions.size()); }
unsigned int AssetManager::GetTriggerCount() { return static_cast<unsigned int>(_triggers.size()); }
unsigned int AssetManager::GetConstantCount() { return static_cast<unsigned int>(_constants.size()); }
unsigned int AssetManager::GetSoundCount() { return static_cast<unsigned int>(_sounds.size()); }
unsigned int AssetManager::GetSpriteCount() { return static_cast<unsigned int>(_sprites.size()); }
unsigned int AssetManager::GetBackgroundCount() { return static_cast<unsigned int>(_backgrounds.size()); }
unsigned int AssetManager::GetPathCount() { return static_cast<unsigned int>(_paths.size()); }
unsigned int AssetManager::GetScriptCount() { return static_cast<unsigned int>(_scripts.size()); }
unsigned int AssetManager::GetFontCount() { return static_cast<unsigned int>(_fonts.size()); }
unsigned int AssetManager::GetTimelineCount() { return static_cast<unsigned int>(_timelines.size()); }
unsigned int AssetManager::GetObjectCount() { return static_cast<unsigned int>(_objects.size()); }
unsigned int AssetManager::GetRoomCount() { return static_cast<unsigned int>(_rooms.size()); }
unsigned int AssetManager::GetIncludeFileCount() { return static_cast<unsigned int>(_includeFiles.size()); }

void AssetManager::CompileObjectIdentities() {
    // Compile object identity lists and children lists
    for (unsigned int i = 0; i < _objects.size(); i++) {
        Object& obj = _objects[i];
        if (!obj.exists) continue;
        obj.identities.insert(i);
        Object* o = &obj;
        while (o->parentIndex >= 0) {
            obj.identities.insert(o->parentIndex);
            o = &_objects[o->parentIndex];
            o->children.insert(i);
        }
    }

    // Compile object parented event lists and identities
    for (unsigned int i = 0; i < _objects.size(); i++) {
        Object& obj = _objects[i];
        if (!obj.exists) continue;

        // event lists
        for (unsigned int j = 0; j < 12; j++) {
            Object* o = &obj;
            while (true) {
                for (const auto& e : o->events[j]) {
                    if (std::find(obj.evList[j].begin(), obj.evList[j].end(), e.first) == obj.evList[j].end()) {
                        obj.evList[j].push_back(e.first);
                    }
                }
                if (o->parentIndex < 0) break;
                o = &_objects[o->parentIndex];
            }
            std::sort(obj.evList[j].begin(), obj.evList[j].end());
            obj.evList[j].shrink_to_fit();
        }
    }

    // Trade collision events between holders and targets, including extended children
    for (unsigned int i = 0; i < _objects.size(); i++) {
        Object& obj = _objects[i];
        if (!obj.exists) continue;

        // Make a copy of the list because it can be bodified during iteration
        std::vector<unsigned int> evList4 = obj.evList[4];
        for(const unsigned int& targetId : evList4) { // For each collision target
            Object& target = _objects[targetId];
            if (!target.exists) continue;

            // update o2 to collide with o1
            if (std::find(target.evList[4].begin(), target.evList[4].end(), i) == target.evList[4].end()) {
                target.evList[4].push_back(i);
            }
            // update all of o1's children to collide with o2 and all of its children
            for(const unsigned int& childId : obj.children) {
                Object& child = _objects[childId];
                // o1 child -> o2
                if (std::find(child.evList[4].begin(), child.evList[4].end(), targetId) == child.evList[4].end()) {
                    child.evList[4].push_back(targetId);
                }
                for (const unsigned int& child2Id : target.children) {
                    // o1 child -> o2 child
                    if (std::find(child.evList[4].begin(), child.evList[4].end(), child2Id) == child.evList[4].end()) {
                        child.evList[4].push_back(child2Id);
                    }
                }
            }
            // update all of o2's children to collide with o1 and all of its children
            for (const unsigned int& childId : target.children) {
                Object& child = _objects[childId];
                // o2 child -> o1
                if (std::find(child.evList[4].begin(), child.evList[4].end(), i) == child.evList[4].end()) {
                    child.evList[4].push_back(i);
                }
                for (const unsigned int& child2Id : obj.children) {
                    // o2 child -> o1 child
                    if (std::find(child.evList[4].begin(), child.evList[4].end(), child2Id) == child.evList[4].end()) {
                        child.evList[4].push_back(child2Id);
                    }
                }
            }
        }
    }

    // Populate event holder lists
    for (unsigned int i = 0; i < _objects.size(); i++) {
        Object& obj = _objects[i];
        if (!obj.exists) continue;
        std::sort(obj.evList[4].begin(), obj.evList[4].end());

        // event lists
        for (unsigned int j = 0; j < 12; j++) {
            for (const unsigned int& e : obj.evList[j]) {
                // Objects only try to collide with objects later in the resource tree - prevents double collision
                if (e <= i) {
                    _eventHolderList[j][e].push_back(i);
                }
            }
        }
    }
}

std::map<unsigned int, std::vector<unsigned int>>& AssetManager::GetEventHolderList(unsigned int ev) {
    return _eventHolderList[ev];
}

std::vector<unsigned int>& AssetManager::GetEventHolderList(unsigned int ev, unsigned int sub) {
    return _eventHolderList[ev][sub];
}
