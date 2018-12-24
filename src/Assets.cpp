#include "Assets.hpp"
#include "CodeActionManager.hpp"

Extension::Extension() {
    name = nullptr;
    folderName = nullptr;
    fileCount = 0;
    files = nullptr;
}

Extension::~Extension() {
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

Trigger::Trigger() {
    name = nullptr;
    exists = true;
    constantName = nullptr;
}

Trigger::~Trigger() {
    free(name);
    free(constantName);
}

Constant::Constant() {
    name = nullptr;
    value = nullptr;
}

Constant::~Constant() {
    free(name);
    free(value);
}

Sound::Sound() {
    name = nullptr;
    exists = true;
    fileType = nullptr;
    fileName = nullptr;
    data = nullptr;
}

Sound::~Sound() {
    free(name);
    free(fileType);
    free(fileName);
    free(data);
}

Sprite::Sprite() {
    name = nullptr;
    exists = true;
    frameCount = 0;
    frames = nullptr;
    collisionMaps = nullptr;
}

Sprite::~Sprite() {
    free(name);

    while (frameCount) {
        frameCount--;
        if (separateCollision || !frameCount) delete[] collisionMaps[frameCount].collision;
    }
    free(frames);
    delete[] collisionMaps;
}

Background::Background() {
    name = nullptr;
    exists = true;
}

Background::~Background() { free(name); }

Path::Path() {
    name = nullptr;
    exists = true;
    points = nullptr;
}

Path::~Path() {
    free(name);
    delete[] points;
}

Script::Script() {
    name = nullptr;
    exists = true;
}

Script::~Script() { free(name); }

Font::Font() {
    name = nullptr;
    exists = true;
    fontName = nullptr;
}

Font::~Font() {
    free(name);
    free(fontName);
}

Timeline::Timeline() {
    name = nullptr;
    exists = true;
    momentCount = 0;
}

Timeline::~Timeline() {
    free(name);
    for (const auto& i : moments) {
        delete[] i.second.actions;
    }
}

Object::Object() {
    name = nullptr;
    exists = true;
}

Object::~Object() {
    free(name);

    for (unsigned int i = 0; i < 12; i++) {
        for (const auto& i : events[i]) {
            delete[] i.second.actions;
        }
    }
}

Room::Room() {
    name = nullptr;
    exists = true;
    caption = nullptr;
    creationCode = 0;
    backgroundCount = 0;
    backgrounds = nullptr;
    viewCount = 0;
    views = nullptr;
    instanceCount = 0;
    instances = nullptr;
    tileCount = 0;
    tiles = nullptr;
}

Room::~Room() {
    free(name);
    free(caption);

    while (instanceCount) {
        instanceCount--;
    }

    delete[] backgrounds;
    delete[] views;
    delete[] instances;
    delete[] tiles;
}

IncludeFile::IncludeFile() {
    filename = nullptr;
    filepath = nullptr;
    exportFolder = nullptr;
    data = nullptr;
    exists = true;
}

IncludeFile::~IncludeFile() {
    free(filename);
    free(filepath);
    free(exportFolder);
    free(data);
}
