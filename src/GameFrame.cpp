#include <pch.h>

#include "Alarm.hpp"
#include "CodeActionManager.hpp"
#include "CodeRunner.hpp"
#include "Collision.hpp"
#include "Game.hpp"
#include "GamePrivateGlobals.hpp"
#include "InputHandler.hpp"
#include "Instance.hpp"
#include "Renderer.hpp"

bool GameLoadRoom(int id) {
    // Check room index is valid
    if (id < 0) return false;
    if (id >= ( int )AssetManager::GetRoomCount()) return false;

    // Get the Room object for the room we're going to
    Room* room = AssetManager::GetRoom(id);

    // Check room exists
    if (!room->exists) return false;

    InstanceList::Iterator iter;
    InstanceHandle i;
    while ((i = iter.Next()) != InstanceList::NoInstance) {
        // run "room end" event for _instances[i]
        unsigned int oIndex = InstanceList::GetInstance(i).object_index;
        Object* o = AssetManager::GetObject(oIndex);
        if (o->events[7].count(5)) {
            if (!CodeActionManager::RunInstanceEvent(7, 5, i, InstanceList::NoInstance, oIndex)) return false;
        }
    }

    // Delete non-persistent instances
    InstanceList::ClearNonPersistent();

    // Clear inputs, because gm8 does this for some reason
    InputClearKeys();

    // Update renderer
    RResizeGameWindow(room->width, room->height);
    RSetGameWindowTitle(room->caption);
    RSetBGColour(room->backgroundColour);

    // Update views
    _globals.view_enabled = room->enableViews;
    _globals.views.clear();
    for (unsigned int i = 0; i < room->viewCount; i++) {
        _globals.views[i].xview = room->views[i].viewX;
        _globals.views[i].yview = room->views[i].viewY;
        _globals.views[i].wview = room->views[i].viewW;
        _globals.views[i].hview = room->views[i].viewH;
        _globals.views[i].xport = room->views[i].portX;
        _globals.views[i].yport = room->views[i].portY;
        _globals.views[i].wport = room->views[i].portW;
        _globals.views[i].hport = room->views[i].portH;
        _globals.views[i].angle = 0.0;
        _globals.views[i].hborder = static_cast<int>(room->views[i].Hbor);
        _globals.views[i].vborder = static_cast<int>(room->views[i].Vbor);
        _globals.views[i].hspeed = static_cast<int>(room->views[i].Hsp);
        _globals.views[i].vspeed = static_cast<int>(room->views[i].Vsp);
        _globals.views[i].object = room->views[i].follow;
        _globals.views[i].visible = room->views[i].visible;
    }

    // Update room
    _globals.room = id;
    _globals.changeRoom = false;
    if (room->speed != _lastUsedRoomSpeed) {
        _globals.room_speed = room->speed;
        _lastUsedRoomSpeed = _globals.room_speed;
    }
    _globals.room_width = room->width;
    _globals.room_height = room->height;

    // Create all instances in new room
    for (unsigned int i = 0; i < room->instanceCount; i++) {
        if (!InstanceList::GetInstanceByNumber(room->instances[i].id)) {
            unsigned int id = room->instances[i].id;
            InstanceHandle instance = InstanceList::AddInstance(id, room->instances[i].x, room->instances[i].y, room->instances[i].objectIndex);
            if (instance == InstanceList::NoInstance) {
                // Failed to create instance
                return false;
            }
            unsigned int oIndex = room->instances[i].objectIndex;
            // run room->instances[i] creation code
            if (!CodeManager::Run(room->instances[i].creation, instance, InstanceList::NoInstance, 11, 32, oIndex)) return false;
            // run instance create event
            if (!CodeActionManager::RunInstanceEvent(0, 0, instance, InstanceList::NoInstance, oIndex)) return false;
        }
    }

    // run room's creation code
    if (!CodeManager::Run(room->creationCode, InstanceList::GetDummyInstance(), InstanceList::NoInstance, 11, 32, 0)) return false;

    iter = InstanceList::Iterator();
    while ((i = iter.Next()) != InstanceList::NoInstance) {
        unsigned int oIndex = InstanceList::GetInstance(i).object_index;
        // run _instance's room start event
        Object* o = AssetManager::GetObject(oIndex);
        if (o->events[7].count(4)) {
            if (!CodeActionManager::RunInstanceEvent(7, 4, i, InstanceList::NoInstance, oIndex)) return false;
        }
    }

    return true;
}


bool GameFrame() {
    InstanceHandle instance;
    InstanceList::Iterator iter;

    // Update inputs from keyboard and mouse (doesn't really matter where this is in the event order as far as I know)
    InputUpdate();

    // TODO: "begin step" trigger events

    // Run "begin step" event for all instances
    for (unsigned int i : GetEventHolderList(3, 1)) {
        iter = InstanceList::Iterator(i);
        while((instance = iter.Next()) != InstanceList::NoInstance) {
            unsigned int oIndex = InstanceList::GetInstance(instance).object_index;
            if(oIndex == i) {
                if (!CodeActionManager::RunInstanceEvent(3, 1, instance, InstanceList::NoInstance, oIndex)) return false;
                if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
            }
        }
    } 

    // TODO: if timeline_running, add timeline_speed to timeline_position and then run any events in that timeline indexed BELOW (not equal to) the current timeline_position
    iter = InstanceList::Iterator();
    while((instance = iter.Next()) != InstanceList::NoInstance) {
        Instance& inst = InstanceList::GetInstance(instance);
        if (inst.timeline_running) {
            Timeline* timeline = AssetManager::GetTimeline(inst.timeline_index);
            if (timeline->exists) {
                double oldTPos = inst.timeline_position;
                inst.timeline_position += inst.timeline_speed;
                double newTPos = inst.timeline_position;

                for (const auto& m : timeline->moments) {
                    if (m.first >= oldTPos && m.first < newTPos) {
                        if (!CodeActionManager::Run(m.second.actions, m.second.actionCount, instance, InstanceList::NoInstance, 0, 0, inst.object_index)) return false;
                    }
                }
            }
        }
    }

    // Subtract from alarms and run event if they reach 0
    AlarmUpdateAll();
    for (const auto& ev : GetEventHolderList(2)) {      // alarm number
        for (const unsigned int& holder : ev.second) {  // event holder
            iter = InstanceList::Iterator(holder);
            while((instance = iter.Next()) != InstanceList::NoInstance) {
                Instance& inst = InstanceList::GetInstance(instance);
                if(AlarmGetMap(inst.id).count(ev.first)) {
                    if (AlarmGetMap(inst.id)[ev.first] == 0) {
                        if (!CodeActionManager::RunInstanceEvent(2, ev.first, instance, instance, inst.object_index)) return false;
                        if (AlarmGet(inst.id, ev.first) == 0) AlarmDelete(inst.id, ev.first);  // Only remove entry if it's still 0
                        if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
                    }
                }
            }
        }
    }

    // Key events
    for (const auto& ev : GetEventHolderList(5)) {      // key number
        for (const unsigned int& holder : ev.second) {  // event holder
            iter = InstanceList::Iterator(holder);
            while((instance = iter.Next()) != InstanceList::NoInstance) {
                if (InputCheckKey(ev.first)) {
                    if (!CodeActionManager::RunInstanceEvent(5, ev.first, instance, InstanceList::NoInstance, InstanceList::GetInstance(instance).object_index)) return false;  // Key event
                    if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
                }
            }
        }
    }

    // TODO: mouse events

    // Key press events
    for (const auto& ev : GetEventHolderList(9)) {      // key number
        for (const unsigned int& holder : ev.second) {  // event holder
            iter = InstanceList::Iterator(holder);
            while((instance = iter.Next()) != InstanceList::NoInstance) {
                if (InputCheckKeyPressed(ev.first)) {
                    if (!CodeActionManager::RunInstanceEvent(9, ev.first, instance, InstanceList::NoInstance, InstanceList::GetInstance(instance).object_index)) return false;  // Key press event
                    if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
                }
            }
        }
    }

    // Key release events
    for (const auto& ev : GetEventHolderList(10)) {     // key number
        for (const unsigned int& holder : ev.second) {  // event holder
            iter = InstanceList::Iterator(holder);
            while((instance = iter.Next()) != InstanceList::NoInstance) {
                if (InputCheckKeyReleased(ev.first)) {
                    if (!CodeActionManager::RunInstanceEvent(10, ev.first, instance, InstanceList::NoInstance, InstanceList::GetInstance(instance).object_index)) return false;  // Key release event
                    if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
                }
            }
        }
    }

    // TODO: "normal step" trigger events

    // Run "step" event for all instances
    for (unsigned int i : GetEventHolderList(3, 0)) {
        iter = InstanceList::Iterator(i);
        while((instance = iter.Next()) != InstanceList::NoInstance) {
            //if (instance->object_index == i) {
                if (!CodeActionManager::RunInstanceEvent(3, 0, instance, InstanceList::NoInstance, InstanceList::GetInstance(instance).object_index)) return false;
                if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
            //}
        }
    } 

    // Movement
    iter = InstanceList::Iterator();
    while((instance = iter.Next()) != InstanceList::NoInstance) {
        Instance& inst = InstanceList::GetInstance(instance);

        // Set xprevious and yprevious
        inst.xprevious = inst.x;
        inst.yprevious = inst.y;

        if (inst.friction != 0) {
            // Subtract friction from speed towards 0
            if(inst.speed < 0) {
                inst.speed += inst.friction;
                if(inst.speed > 0) inst.speed = 0;
            }
            else {
                inst.speed -= inst.friction;
                if (inst.speed < 0) inst.speed = 0;
            }

            // Recalculate hspeed/vspeed
            inst.hspeed = cos(inst.direction * GML_PI / 180.0) * inst.speed;
            inst.vspeed = -sin(inst.direction * GML_PI / 180.0) * inst.speed;
        }

        if (inst.gravity) {
            // Apply gravity in gravity_direction to hspeed and vspeed
            inst.hspeed += cos(inst.gravity_direction * GML_PI / 180.0) * inst.gravity;
            inst.vspeed += -sin(inst.gravity_direction * GML_PI / 180.0) * inst.gravity;

            // Recalculate speed and direction from hspeed/vspeed
            inst.direction = ::atan2(-inst.vspeed, inst.hspeed) * 180.0 / GML_PI;
            inst.speed = sqrt(pow(inst.hspeed, 2) + pow(inst.vspeed, 2));
        }

        // Apply hspeed and vspeed to x and y
        inst.x += inst.hspeed;
        inst.y += inst.vspeed;
        if (inst.hspeed || inst.vspeed) inst.bboxIsStale = true;
    }

    // Outside Room event
    for (unsigned int i : GetEventHolderList(7, 0)) {
        iter = InstanceList::Iterator(i);
        while((instance = iter.Next()) != InstanceList::NoInstance) {
            //if (instance->object_index == i) {
                Instance& inst = InstanceList::GetInstance(instance);

                RefreshInstanceBbox(&inst);
                if ((inst.sprite_index < 0)
                        ? (inst.x < 0 || inst.y < 0 || inst.x > ( int )_globals.room_width || inst.y > ( int )_globals.room_height)
                        : (inst.bbox_bottom < 0 || inst.bbox_right < 0 || inst.bbox_top > ( int )_globals.room_height || inst.bbox_left > ( int )_globals.room_width)) {
                    if (!CodeActionManager::RunInstanceEvent(7, 0, instance, InstanceList::NoInstance, inst.object_index)) return false;
                    if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
                }
            //}
        }
    }

    // Intersect boundary event
    for (unsigned int i : GetEventHolderList(7, 1)) {
        iter = InstanceList::Iterator(i);
        while((instance = iter.Next()) != InstanceList::NoInstance) {
            Instance& inst = InstanceList::GetInstance(instance);
            //if (instance->object_index == i) {
                RefreshInstanceBbox(&inst);
                if (inst.bbox_bottom > ( int )_globals.room_height || inst.bbox_right > ( int )_globals.room_width || inst.bbox_top < 0 || inst.bbox_left < 0) {
                    if (!CodeActionManager::RunInstanceEvent(7, 1, instance, InstanceList::NoInstance, inst.object_index)) return false;
                    if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
                }
            //}
        }
    }

    // TODO: in this order, if views are enabled: "outside view x" events for all instances, "intersect boundary view x" events for all instances

    // Collision events
    for (const auto& ev : GetEventHolderList(4)) {      // target object id
        for (const unsigned int& holder : ev.second) {  // event holder
            iter = InstanceList::Iterator(holder);
            while((instance = iter.Next()) != InstanceList::NoInstance) {
                Instance& inst1 = InstanceList::GetInstance(instance);

                InstanceList::Iterator iter2(ev.first, instance);
                InstanceHandle instance2;
                while ((instance2 = iter2.Next()) != InstanceList::NoInstance) {
                    if (instance != instance2) {
                        Instance& inst2 = InstanceList::GetInstance(instance2);
                        if (CollisionCheck(&inst1, &inst2)) {

                            // self->other
                            if (inst2.solid) {
                                // If the target is solid, we move outside of it
                                inst1.x = inst1.xprevious;
                                inst1.y = inst1.yprevious;
                                inst1.bboxIsStale = true;
                            }
                            if (!CodeActionManager::RunInstanceEvent(4, ev.first, instance, instance2, inst1.object_index)) return false;
                            if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);

                            if (inst2.solid) {
                                inst1.x += inst1.hspeed;
                                inst1.y += inst1.vspeed;
                                inst1.bboxIsStale = true;
                            }

                            // other->self
                            if (inst1.solid) {
                                // If the target is solid, we move outside of it
                                inst2.x = inst2.xprevious;
                                inst2.y = inst2.yprevious;
                                inst2.bboxIsStale = true;
                            }
                            if (!CodeActionManager::RunInstanceEvent(4, holder, instance2, instance, inst2.object_index)) return false;
                            if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);

                            if (inst1.solid) {
                                inst2.x += inst2.hspeed;
                                inst2.y += inst2.vspeed;
                                inst2.bboxIsStale = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // TODO: "end step" trigger events

    // Run "end step" event for all instances
    for (unsigned int i : GetEventHolderList(3, 2)) {
        iter = InstanceList::Iterator(i);
        while((instance = iter.Next()) != InstanceList::NoInstance) {
            //if (instance->object_index == i) {
                if (!CodeActionManager::RunInstanceEvent(3, 2, instance, InstanceList::NoInstance, InstanceList::GetInstance(instance).object_index)) return false;
                if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
            //}
        }
    } 

    // Clear deleted instances from InstanceList
    // NB: this must be done here and nowhere else so that instance_count is reported correctly
    InstanceList::ClearDeleted();

    // Prepare screen for drawing
    RStartFrame();

    // Draw room backgrounds
    Room* room = AssetManager::GetRoom(_globals.room);
    for (unsigned int i = 0; i < room->backgroundCount; i++) {
        RoomBackground bg = room->backgrounds[i];
        if (bg.visible && !bg.foreground && bg.backgroundIndex >= 0) {
            Background* b = AssetManager::GetBackground(bg.backgroundIndex);
            if (b->exists) {
                unsigned int stretchedW = (bg.stretch ? room->width : b->width);
                unsigned int stretchedH = (bg.stretch ? room->height : b->height);
                double scaleX = (bg.stretch ? (( double )room->width / b->width) : 1);
                double scaleY = (bg.stretch ? (( double )room->height / b->height) : 1);

                for (int startY = (bg.tileVert ? (bg.y - stretchedH) : 0); startY < ( int )room->height; startY += stretchedH) {
                    for (int startX = (bg.tileHor ? (bg.x - stretchedW) : 0); startX < ( int )room->width; startX += stretchedW) {
                        RDrawImage(b->image, startX, startY, scaleX, scaleY, 0, 0xFFFFFFFF, 1, INT_MAX);
                    }
                }
            }
        }
    }

    // Draw all tiles
    for (unsigned int i = 0; i < room->tileCount; i++) {
        RoomTile tile = room->tiles[i];
        RDrawPartialImage(AssetManager::GetBackground(tile.backgroundIndex)->image, tile.x, tile.y, 1, 1, 0, 0xFFFFFFFF, 1, tile.tileX, tile.tileY, tile.width, tile.height, tile.depth);
    }

    // Run draw event for all instances in depth order
    int nextDepth = INT_MIN;
    iter = InstanceList::Iterator();
    while((instance = iter.Next()) != InstanceList::NoInstance) {
        Instance& inst = InstanceList::GetInstance(instance);
        if (inst.depth > nextDepth && inst.exists && inst.visible) nextDepth = inst.depth;
    }

    while (true) {
        int currentDepth = nextDepth;
        nextDepth = INT_MIN;
        iter = InstanceList::Iterator();
        while((instance = iter.Next()) != InstanceList::NoInstance) {
            Instance& inst = InstanceList::GetInstance(instance);

            // Don't run draw event for instances that don't exist or aren't visible.
            if (inst.visible) {
                if (inst.depth == currentDepth) {
                    Object* obj = AssetManager::GetObject(inst.object_index);
                    if (obj->events[8].count(0)) {
                        // This object has a custom draw event.
                        if (!CodeActionManager::RunInstanceEvent(8, 0, instance, InstanceList::NoInstance, inst.object_index)) return false;
                        if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
                    }
                    else {
                        // This is the default draw action if no draw event is present for this object.
                        if (inst.sprite_index >= 0) {
                            Sprite* sprite = AssetManager::GetSprite(inst.sprite_index);
                            if (sprite->exists) {
                                RDrawImage(sprite->frames[(( int )inst.image_index) % sprite->frameCount], inst.x, inst.y, inst.image_xscale, inst.image_yscale,
                                    inst.image_angle, inst.image_blend, inst.image_alpha, inst.depth);
                            }
                            else {
                                // Tried to draw non-existent sprite
                                return false;
                            }
                        }
                    }
                }
                else {
                    if (inst.depth < currentDepth && inst.depth > nextDepth) nextDepth = inst.depth;
                }
            }
        }
        if (nextDepth == INT_MIN) break;
    }

    // Draw room foregrounds
    for (unsigned int i = 0; i < room->backgroundCount; i++) {
        RoomBackground bg = room->backgrounds[i];
        if (bg.visible && bg.foreground) {
            Background* b = AssetManager::GetBackground(bg.backgroundIndex);
            unsigned int stretchedW = (bg.stretch ? room->width : b->width);
            unsigned int stretchedH = (bg.stretch ? room->height : b->height);
            double scaleX = (bg.stretch ? (( double )room->width / b->width) : 1);
            double scaleY = (bg.stretch ? (( double )room->height / b->height) : 1);

            for (int startY = (bg.tileVert ? (bg.y - stretchedH) : 0); startY < ( int )room->height; startY += stretchedH) {
                for (int startX = (bg.tileHor ? (bg.x - stretchedW) : 0); startX < ( int )room->width; startX += stretchedW) {
                    RDrawImage(b->image, startX, startY, scaleX, scaleY, 0, 0xFFFFFFFF, 1, INT_MIN);
                }
            }
        }
    }

    // Draw screen
    RRenderFrame();

    // Update Caption
    RSetGameWindowTitle(_globals.room_caption.c_str());
    if (RShouldClose()) return false;

    // Update sprite info
    iter = InstanceList::Iterator();
    while((instance = iter.Next()) != InstanceList::NoInstance) {
        Instance& inst = InstanceList::GetInstance(instance);
        inst.image_index += inst.image_speed;

        if (inst.sprite_index >= 0) {
            Sprite* s = AssetManager::GetSprite(inst.sprite_index);
            if (inst.image_index >= s->frameCount) {
                inst.image_index -= s->frameCount;
                if (!CodeActionManager::RunInstanceEvent(7, 7, instance, InstanceList::NoInstance, inst.object_index)) return false;  // Animation End event
                if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
            }
            if (inst.image_speed && s->separateCollision) inst.bboxIsStale = true;
        }
    }

    return true;
}
