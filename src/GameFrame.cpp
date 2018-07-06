#include "Game.hpp"
#include "Renderer.hpp"
#include "Instance.hpp"
#include "CodeActionManager.hpp"
#include "CodeRunner.hpp"
#include "InputHandler.hpp"
#include "Collision.hpp"
#include "GamePrivateGlobals.hpp"


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
		if (o->evOther.count(5)) {
			if (!_codeActions->Run(o->evOther[5].actions, o->evOther[5].actionCount, _instances[i], NULL)) return false;
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
			if (!_runner->Run(room->instances[i].creation, instance, NULL)) return false;
			// run instance create event
			Object* o = AMGetObject(instance->object_index);
			if (!_codeActions->Run(o->evCreate, o->evCreateActionCount, instance, NULL)) return false;
		}
	}

	// run room's creation code
	if (!_runner->Run(room->creationCode, NULL, NULL)) return false;

	count = _instances.Count();
	for (unsigned int i = 0; i < count; i++) {
		// run _instances[i] room start event
		Object* o = AMGetObject(_instances[i]->object_index);
		if (o->evOther.count(4)) {
			if (!_codeActions->Run(o->evOther[4].actions, o->evOther[4].actionCount, _instances[i], NULL)) return false;
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
		if (!_codeActions->Run(o->evStepBegin, o->evStepBeginActionCount, instance, NULL)) return false;
		if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
	}

	// TODO: if timeline_running, add timeline_speed to timeline_position and then run any events in that timeline indexed BELOW (not equal to) the current timeline_position

	// Subtract from alarms and run event if they reach 0
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* obj = AMGetObject(instance->object_index);
		for (auto const& j : instance->alarm) {
			if (j.second > 0) {
				instance->alarm[j.first]--;
				if (instance->alarm[j.first] == 0) {
					if (!_codeActions->Run(obj->evAlarm[j.first].actions, obj->evAlarm[j.first].actionCount, instance, NULL)) return false;
					if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
				}
			}
		}
	}

	// TODO: keyboard events

	// TODO: mouse events

	// TODO: key press events

	// TODO: key release events

	// TODO: "normal step" trigger events

	// Run "step" event for all instances
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* o = AMGetObject(instance->object_index);
		if (!_codeActions->Run(o->evStep, o->evStepActionCount, instance, NULL)) return false;
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
		for (const auto& e : o->evCollision) {
			InstanceList::Iterator iter(&_instances, e.first);

			Instance* target = iter.Next();
			while (target) {
				if (target != instance) {
					if (CollisionCheck(instance, target)) {
						if (!_codeActions->Run(e.second.actions, e.second.actionCount, instance, target)) return false;
						if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
					}
				}
				target = iter.Next();
			}
		}
	}

	// TODO: "end step" trigger events

	// Run "end step" event for all instances
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		Object* o = AMGetObject(instance->object_index);
		if (!_codeActions->Run(o->evStepEnd, o->evStepEndActionCount, instance, NULL)) return false;
		if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
	}

	// Prepare screen for drawing
	RStartFrame();

	// Run draw event for all instances (TODO: correct depth order)
	unsigned int icount = _instances.Count();
	for (unsigned int i = 0; i < icount; i++) {
		Instance* instance = _instances[i];
		// Don't run draw event for instances that don't exist or aren't visible.
		if (instance->exists && instance->visible) {

			Object* obj = AMGetObject(instance->object_index);
			if (obj->evDraw) {
				// This object has a custom draw event.
				if (!_codeActions->Run(obj->evDraw, obj->evDrawActionCount, instance, NULL)) return false;
				if (_globals.changeRoom) return GameLoadRoom(_globals.roomTarget);
			}
			else {
				// This is the default draw action if no draw event is present for this object.
				if (instance->sprite_index >= 0) {
					Sprite* sprite = AMGetSprite(instance->sprite_index);
					if (sprite->exists) {
						RDrawImage(sprite->frames[((int)instance->image_index) % sprite->frameCount], instance->x, instance->y, instance->image_xscale, instance->image_yscale, instance->image_angle, instance->image_blend, instance->image_alpha);
					}
					else {
						// Tried to draw non-existent sprite
						return false;
					}
				}
			}
		}
	}

	// Draw screen
	RRenderFrame();
	if (RShouldClose()) return false;

	// Update sprite info
	iter = InstanceList::Iterator(&_instances);
	while (instance = iter.Next()) {
		instance->image_index += instance->image_speed;

		if (instance->sprite_index >= 0) {
			Sprite* s = AMGetSprite(instance->sprite_index);
			if (instance->image_index > s->frameCount) {
				instance->image_index -= s->frameCount;
			}
			if (instance->image_speed && s->separateCollision) instance->bboxIsStale = true;
		}
	}

	_instances.ClearDeleted();

	return true;
}