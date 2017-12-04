#include "Game.hpp"
#include "GameRenderer.hpp"
#include "Instance.hpp"
#include "CodeAction.hpp"
#include "CodeRunner.hpp"

bool Game::_runActions(CodeAction* actions, unsigned int count, Instance* self, Instance* other) {
	unsigned int pos = 0;
	while (pos < count) {
		bool run = true;
		if (actions[pos].IsQuestion()) {
			// Multiple questions can be chained together with the end statement dependent on all of them.
			while (actions[pos].IsQuestion()) {
				if (run) {
					bool r;
					if (!actions[pos].Run(_runner, self, other, &r)) return false;
					run &= r;
				}
				pos++;
			}

		}
		if(run) {
			// This action isn't a question and we are going to run it.

			switch (actions[pos].getActID()) {
				case 422: {
					// "start block", so run until the matching end
					pos++;
					CodeAction* start = actions + pos;
					unsigned int depth = 1;
					unsigned int count = 0;
					while (depth) {
						if (actions[pos].getActID() == 422) depth++;
						else if (actions[pos].getActID() == 424) depth--;
						count++;
						pos++;
					}
					count--; // Skips running the "end block" itself
					_runActions(start, count, self, other);
					break;
				}
				case 605: {
					// Comment, don't bother doing anything
					break;
				}
				default: {
					if (!actions[pos].Run(_runner, self, other)) return false;
					pos++;
					break;
				}
			}
		}
		else {
			// Not a question and we are NOT going to run it.
			switch (actions[pos].getActID()) {
				case 422: {
					// "start block", so skip until matching end block
					pos++;
					unsigned int depth = 1;
					while (depth) {
						if (actions[pos].getActID() == 422) depth++;
						else if (actions[pos].getActID() == 424) depth--;
						pos++;
					}
					break;
				}
				default: {
					pos++;
					break;
				}
			}
		}
	}
}



bool Game::LoadRoom(unsigned int id) {
	// Exit if we're already in this room
	if (id == _globals.room) return true;

	// Get the Room object for the room we're going to
	Room* room = _assetManager.GetRoom(id);

	// Check room exists
	if (!room->exists) return false;

	for (unsigned int i = 0; i < _instances.Count(); i++) {
		// run "room end" event for _instances[i]
		Object* o = _assetManager.GetObject(_instances[i]->object_index);
		for (IndexedEvent e : o->evOther) {
			if (e.index == 5) {
				_runActions(e.actions, e.actionCount, _instances[i], NULL);
				break;
			}
		}
	}

	// Delete non-persistent instances
	_instances.ClearNonPersistent();

	// Update renderer
	_renderer->ResizeGameWindow(room->width, room->height);
	_renderer->SetGameWindowTitle(room->caption);

	// Update room
	_globals.room = id;
	if (room->speed != _lastUsedRoomSpeed) {
		_globals.room_speed = room->speed;
		_lastUsedRoomSpeed = _globals.room_speed;
		_globals.room_width = room->width;
		_globals.room_height = room->height;
	}

	// Create all instances in new room
	for (unsigned int i = 0; i < room->instanceCount; i++) {
		if (!_instances.GetInstanceByNumber(room->instances[i].id)) {
			Instance* instance = _instances.AddInstance(room->instances[i].id, room->instances[i].x, room->instances[i].y, room->instances[i].objectIndex);
			if (!instance) {
				// Failed to create instance
				return false;
			}
			// run room->instances[i] creation code
			if (!_runner->Run(room->instances[i].creation, instance, NULL)) return false;
			// run instance create event
			Object* o = _assetManager.GetObject(instance->object_index);
			if (!_runActions(o->evCreate, o->evCreateActionCount, instance, NULL)) return false;
		}
	}

	// run room's creation code
	if (!_runner->Run(room->creationCode, NULL, NULL)) return false;

	for (unsigned int i = 0; i < _instances.Count(); i++) {
		// run _instances[i] room start event
		Object* o = _assetManager.GetObject(_instances[i]->object_index);
		for (IndexedEvent e : o->evOther) {
			if (e.index == 4) {
				_runActions(e.actions, e.actionCount, _instances[i], NULL);
				break;
			}
		}
	}

	return true;
}



bool Game::Frame() {
	for (unsigned int i = 0; i < _instances.Count(); i++) {
		Instance* instance = _instances[i];

		// This is the default draw action if no draw event is present for this object (more or less.)
		// We can just do this for now until we have code compilation working.
		if (instance->sprite_index >= 0 && instance->visible) {
			Sprite* sprite = _assetManager.GetSprite(instance->sprite_index);
			if (sprite->exists) {
				_renderer->DrawImage(sprite->frames[((int)instance->image_index) % sprite->frameCount], instance->x, instance->y, instance->image_xscale, instance->image_yscale, instance->image_angle, instance->image_blend, instance->image_alpha);
			}
			else {
				// Tried to draw non-existent sprite
				return false;
			}
		}

		// One of the many things that happens during the frame cycle - using this to test animation.
		instance->image_index += instance->image_speed;
	}

	_renderer->RenderFrame();
	return !_renderer->ShouldClose();
}