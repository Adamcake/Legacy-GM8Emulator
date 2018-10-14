#include <pch.h>
#include "Game.hpp"
#include "Renderer.hpp"
#include "Instance.hpp"
#include "CodeActionManager.hpp"
#include "CodeRunner.hpp"
#include "InputHandler.hpp"
#include "Collision.hpp"
#include "GamePrivateGlobals.hpp"
#include "Alarm.hpp"

bool GameLoadRoom(int id) {
	// Check room index is valid
	if (id < 0) return false;
	if (id >= (int)AMGetRoomCount()) return false;

	// Get the Room object for the room we're going to
	Room* room = AMGetRoom(id);

	// Check room exists
	if (!room->exists) return false;

	unsigned int count = _instances.Count();
	for (unsigned int i = 0; i < count; i++) {
		// run "room end" event for _instances[i]
		Object* o = AMGetObject(_instances[i]->object_index);
		if (o->events[7].count(5)) {
			if (!_codeActions->RunInstanceEvent(7, 5, _instances[i], NULL, _instances[i]->object_index)) return false;
		}
	}

	// Delete non-persistent instances
	_instances.ClearNonPersistent();

	// Clear inputs, because gm8 does this for some reason
	InputClearKeys();

	// Update renderer
	RResizeGameWindow(room->width, room->height);
	RSetGameWindowTitle(room->caption);
	RSetBGColour(room->backgroundColour);

	// Update room
	_globals.room = id;
	_globals.changeRoom = false;
	if (room->speed != _lastUsedRoomSpeed) {
		_globals.room_speed = room->speed;
		_lastUsedRoomSpeed = _globals.room_speed;
		_globals.room_width = room->width;
		_globals.room_height = room->height;
	}

	// Create all instances in new room
	for (unsigned int i = 0; i < room->instanceCount; i++) {
		if (!_instances.GetInstanceByNumber(room->instances[i].id)) {
			unsigned int id = room->instances[i].id;
			Instance* instance = _instances.AddInstance(id, room->instances[i].x, room->instances[i].y, room->instances[i].objectIndex);
			if (!instance) {
				// Failed to create instance
				return false;
			}
			// run room->instances[i] creation code
			if (!_runner->Run(room->instances[i].creation, instance, NULL, 0, 0, 0)) return false; // not sure if it matters what event id and number I pass here?
			// run instance create event
			Object* o = AMGetObject(instance->object_index);
			if (!_codeActions->RunInstanceEvent(0, 0, instance, NULL, instance->object_index)) return false;
		}
	}

	// run room's creation code
	if (!_runner->Run(room->creationCode, NULL, NULL, 0, 0, 0)) return false; // not sure if it matters what event id and number I pass here

	InstanceList::Iterator iter(&_instances);
	Instance* instance;
	while (instance = iter.Next()) {
		// run _instance's room start event
		Object* o = AMGetObject(instance->object_index);
		if (o->events[7].count(4)) {
			if (!_codeActions->RunInstanceEvent(7, 4, instance, NULL, instance->object_index)) return false;
		}
	}

	return true;
}



bool GameFrame() {
	Instance* instance;
	InstanceList::Iterator iter(&_instances);

	// Update inputs from keyboard and mouse (doesn't really matter where this is in the event order as far as I know)
	InputUpdate();

	// TODO: "begin step" trigger events

	// Run "begin step" event for all instances
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* o = AMGetObject(instance->object_index);
		if (!_codeActions->RunInstanceEvent(3, 1, instance, NULL, instance->object_index)) return false;
		if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
	}

	// TODO: if timeline_running, add timeline_speed to timeline_position and then run any events in that timeline indexed BELOW (not equal to) the current timeline_position
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		if (instance->timeline_running) {
			Timeline* timeline = AMGetTimeline(instance->timeline_index);
			if (timeline->exists) {
				double oldTPos = instance->timeline_position;
				instance->timeline_position += instance->timeline_speed;

				for (const auto& m : timeline->moments) {
					if (m.first >= oldTPos && m.first < instance->timeline_position) {
						if (!_codeActions->Run(m.second.actions, m.second.actionCount, instance, NULL, 0, 0, instance->object_index)) return false;
					}
				}
			}
		}
	}

	// Subtract from alarms and run event if they reach 0
	AlarmUpdateAll();
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		for (const auto j : AlarmGetMap(instance->id)) {
			if (j.second == 0) {
				if (!_codeActions->RunInstanceEvent(2, j.first, instance, NULL, instance->object_index)) return false;
				if (AlarmGet(instance->id, j.first) == 0) AlarmDelete(instance->id, j.first); // Only remove entry if it's still 0
				if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
			}
		}
	}

	// Key events
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* o = AMGetObject(instance->object_index);
		for (unsigned int e : o->evList[5]) {
			if (InputCheckKey(e)) {
				if (!_codeActions->RunInstanceEvent(5, e, instance, NULL, instance->object_index)) return false; // Animation End event
				if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
			}
		}
	}

	// TODO: mouse events

	// Key press events
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* o = AMGetObject(instance->object_index);
		for (unsigned int e : o->evList[9]) {
			if (InputCheckKeyPressed(e)) {
				if (!_codeActions->RunInstanceEvent(9, e, instance, NULL, instance->object_index)) return false; // Animation End event
				if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
			}
		}
	}

	// Key release events
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* o = AMGetObject(instance->object_index);
		for (unsigned int e : o->evList[10]) {
			if (InputCheckKey(e)) {
				if (!_codeActions->RunInstanceEvent(10, e, instance, NULL, instance->object_index)) return false; // Animation End event
				if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
			}
		}
	}

	// TODO: "normal step" trigger events

	// Run "step" event for all instances
	iter = InstanceList::Iterator(&_instances);
	unsigned int a = 0;
	while (instance = iter.Next()) {
		if (!_codeActions->RunInstanceEvent(3, 0, instance, NULL, instance->object_index)) return false;
		if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
	}

	// Movement
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		// Set xprevious and yprevious
		instance->xprevious = instance->x;
		instance->yprevious = instance->y;

		if (instance->friction != 0) {
			// Subtract friction from speed towards 0
			bool neg = instance->speed < 0;
			instance->speed = abs(instance->speed) - instance->friction;
			if (instance->speed < 0) instance->speed = 0;
			else if (neg) instance->speed = -instance->speed;

			// Recalculate hspeed/vspeed
			instance->hspeed = cos(instance->direction * PI / 180.0) * instance->speed;
			instance->vspeed = -sin(instance->direction * PI / 180.0) * instance->speed;
		}

		if (instance->gravity) {
			// Apply gravity in gravity_direction to hspeed and vspeed
			instance->hspeed += cos(instance->gravity_direction * PI / 180.0) * instance->gravity;
			instance->vspeed += -sin(instance->gravity_direction * PI / 180.0) * instance->gravity;

			// Recalculate speed and direction from hspeed/vspeed
			instance->direction = atan(-instance->vspeed / instance->hspeed) * 180.0 / PI;
			instance->speed = sqrt(pow(instance->hspeed, 2) + pow(instance->vspeed, 2));
		}

		// Apply hspeed and vspeed to x and y
		instance->x += instance->hspeed;
		instance->y += instance->vspeed;
		if (instance->hspeed || instance->vspeed) instance->bboxIsStale = true;
	}

	// TODO: in this order: "outside room" events for all instances, "intersect boundary" events for all instances
	// TODO: in this order, if views are enabled: "outside view x" events for all instances, "intersect boundary view x" events for all instances

	// Collision events
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* o = AMGetObject(instance->object_index);
		for (unsigned int e : o->evList[4]) {
			InstanceList::Iterator iter2(&_instances, e);
	
			Instance* target = iter2.Next();
			while (target) {
				if (target != instance) {
					if (CollisionCheck(instance, target)) {
						if (target->solid) {
							// If the target is solid, we move outside of it
							instance->x = instance->xprevious;
							instance->y = instance->yprevious;
							instance->bboxIsStale = true;
						}
						if (!_codeActions->RunInstanceEvent(4, e, instance, target, instance->object_index)) return false;
						if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);

						if (target->solid) {
							instance->x += instance->hspeed;
							instance->y += instance->vspeed;
							instance->bboxIsStale = true;
						}
					}
				}
				target = iter2.Next();
			}
		}
	}

	// TODO: "end step" trigger events

	// Run "end step" event for all instances
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		if (!_codeActions->RunInstanceEvent(3, 2, instance, NULL, instance->object_index)) return false;
		if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
	}

	// Prepare screen for drawing
	RStartFrame();

	// Draw room backgrounds
	Room* room = AMGetRoom(_globals.room);
	for (unsigned int i = 0; i < room->backgroundCount; i++) {
		RoomBackground bg = room->backgrounds[i];
		if (bg.visible && !bg.foreground && bg.backgroundIndex >= 0) {
			Background* b = AMGetBackground(bg.backgroundIndex);
			if (b->exists) {
				unsigned int stretchedW = (bg.stretch ? room->width : b->width);
				unsigned int stretchedH = (bg.stretch ? room->height : b->height);
				double scaleX = (bg.stretch ? ((double)room->width / b->width) : 1);
				double scaleY = (bg.stretch ? ((double)room->height / b->height) : 1);

				for (int startY = (bg.tileVert ? (bg.y - stretchedH) : 0); startY < (int)room->height; startY += stretchedH) {
					for (int startX = (bg.tileHor ? (bg.x - stretchedW) : 0); startX < (int)room->width; startX += stretchedW) {
						RDrawImage(b->image, startX, startY, scaleX, scaleY, 0, 0xFFFFFFFF, 1, INT_MAX);
					}
				}
			}			
		}
	}

	// Draw all tiles
	for (unsigned int i = 0; i < room->tileCount; i++) {
		RoomTile tile = room->tiles[i];
		RDrawPartialImage(AMGetBackground(tile.backgroundIndex)->image, tile.x, tile.y, 1, 1, 0, 0xFFFFFFFF, 1, tile.tileX, tile.tileY, tile.width, tile.height, tile.depth);
	}

	// Run draw event for all instances in depth order
	int nextDepth = INT_MIN;
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		if (instance->depth > nextDepth && instance->exists && instance->visible) nextDepth = instance->depth;
	}

	while (true) {
		int currentDepth = nextDepth;
		nextDepth = INT_MIN;
		iter = InstanceList::Iterator(&_instances);
		while (instance = iter.Next()) {
			// Don't run draw event for instances that don't exist or aren't visible.
			if (instance->visible) {
				if (instance->depth == currentDepth) {
					Object* obj = AMGetObject(instance->object_index);
					if (obj->events[8].count(0)) {
						// This object has a custom draw event.
						if (!_codeActions->RunInstanceEvent(8, 0, instance, NULL, instance->object_index)) return false;
						if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
					}
					else {
						// This is the default draw action if no draw event is present for this object.
						if (instance->sprite_index >= 0) {
							Sprite* sprite = AMGetSprite(instance->sprite_index);
							if (sprite->exists) {
								RDrawImage(sprite->frames[((int)instance->image_index) % sprite->frameCount], instance->x, instance->y, instance->image_xscale, instance->image_yscale, instance->image_angle, instance->image_blend, instance->image_alpha, instance->depth);
							}
							else {
								// Tried to draw non-existent sprite
								return false;
							}
						}
					}
				}
				else {
					if (instance->depth < currentDepth && instance->depth > nextDepth) nextDepth = instance->depth;
				}
			}
		}
		if (nextDepth == INT_MIN) break;
	}

	// Draw room foregrounds
	for (unsigned int i = 0; i < room->backgroundCount; i++) {
		RoomBackground bg = room->backgrounds[i];
		if (bg.visible && bg.foreground) {
			Background* b = AMGetBackground(bg.backgroundIndex);
			unsigned int stretchedW = (bg.stretch ? room->width : b->width);
			unsigned int stretchedH = (bg.stretch ? room->height : b->height);
			double scaleX = (bg.stretch ? ((double)room->width / b->width) : 1);
			double scaleY = (bg.stretch ? ((double)room->height / b->height) : 1);

			for (int startY = (bg.tileVert ? (bg.y - stretchedH) : 0); startY < (int)room->height; startY += stretchedH) {
				for (int startX = (bg.tileHor ? (bg.x - stretchedW) : 0); startX < (int)room->width; startX += stretchedW) {
					RDrawImage(b->image, startX, startY, scaleX, scaleY, 0, 0xFFFFFFFF, 1, instance->depth);
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
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		instance->image_index += instance->image_speed;

		if (instance->sprite_index >= 0) {
			Sprite* s = AMGetSprite(instance->sprite_index);
			if (instance->image_index >= s->frameCount) {
				instance->image_index -= s->frameCount;
				if (!_codeActions->RunInstanceEvent(7, 7, instance, NULL, instance->object_index)) return false; // Animation End event
				if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
			}
			if (instance->image_speed && s->separateCollision) instance->bboxIsStale = true;
		}
	}

	_instances.ClearDeleted();

	return true;
}
