#include "InstanceList.hpp"
#include "Instance.hpp"
#include "AssetManager.hpp"
#include <cstring>
#define INSTANCE_CAPACITY 65536


InstanceList::InstanceList(AssetManager* manager) {
	_list = new Instance[INSTANCE_CAPACITY];
	_size = 0;
	_highestIdAdded = 0;
	_assetManager = manager;
}

InstanceList::~InstanceList() {
	delete[] _list;
}

Instance* InstanceList::AddInstance(unsigned int id, double x, double y, unsigned int objectId) {
	if (_highestIdAdded > id) {
		for (unsigned int i = 0; i < _size; i++) {
			if (_list[i].id < id) {
				memmove(_list + i + 1, _list + i, sizeof(Instance) * (_size - i));
				_size++;
				Instance* ret = _list + i;
				if (_InitInstance(ret, id, x, y, objectId)) {
					return ret;
				}
				else return NULL;
			}
		}
	}
	Instance* ret = _list + _size;
	_highestIdAdded = id;
	_size++;
	if (_InitInstance(ret, id, x, y, objectId)) {
		return ret;
	}
	else return NULL;
}

void InstanceList::DeleteInstance(unsigned int id) {
	for (unsigned int i = 0; i < _size; i++) {
		if (_list[i].id < id) {
			memmove(_list + i, _list + i + 1, sizeof(Instance) * (_size - (i + 1)));
			_size--;
			return;
		}
	}
}

void InstanceList::ClearAll() {
	_size = 0;
}

void InstanceList::ClearNonPersistent() {
	unsigned int placed = 0;
	for (unsigned int i = 0; i < _size; i++) {
		if (_list[i].persistent && _list[i].exists) {
			if (placed != i)
				_list[placed] = _list[i];
			placed++;
		}
	}
	_size = placed;
}

void InstanceList::ClearDeleted() {
	unsigned int placed = 0;
	for (unsigned int i = 0; i < _size; i++) {
		if (_list[i].exists) {
			if(placed != i)
				_list[placed] = _list[i];
			placed++;
		}
	}
	_size = placed;
}

Instance * InstanceList::GetInstanceByNumber(unsigned int num, unsigned int startPos, unsigned int* endPos) {
	if (num > 100000) {
		// Instance ID
		for (unsigned int i = startPos; i < _size; i++) {
			if (_list[i].id == num) {
				if(endPos) (*endPos) = i;
				return (_list[i].exists) ? (_list + i) : NULL;
			}
			if (_list[i].id > num) {
				return NULL;
			}
		}
	}
	else {
		// Object ID
		for (unsigned int i = startPos; i < _size; i++) {
			if (_list[i].object_index == num && _list[i].exists) {
				if (endPos) (*endPos) = i;
				return _list + i;
			}
		}
	}
	return NULL;
}

Instance * InstanceList::operator[](unsigned int index) {
	return _list + index;
}



// Private

bool InstanceList::_InitInstance(Instance* instance, unsigned int id, double x, double y, unsigned int objectId) {
	Object* obj = _assetManager->GetObject(objectId);
	if (!obj->exists) return false;
	instance->exists = true;
	instance->id = id;

	instance->object_index = objectId;
	instance->solid = obj->solid;
	instance->visible = obj->visible;
	instance->persistent = obj->persistent;
	instance->depth = obj->depth;
	instance->alarm.clear();
	instance->sprite_index = obj->spriteIndex;
	instance->image_alpha = 1;
	instance->image_blend = 0xFFFFFF;
	instance->image_index = 0;
	instance->image_speed = 1;
	instance->image_xscale = 1;
	instance->image_yscale = 1;
	instance->image_angle = 0;
	instance->mask_index = obj->maskIndex;
	instance->direction = 0;
	instance->gravity = 0;
	instance->gravity_direction = 270;
	instance->hspeed = 0;
	instance->vspeed = 0;
	instance->speed = 0;
	instance->friction = 0;
	instance->x = x;
	instance->y = y;
	instance->xprevious = x;
	instance->yprevious = y;
	instance->xstart = x;
	instance->ystart = y;
	instance->path_index = -1;
	instance->path_position = 0;
	instance->path_positionprevious = 0;
	instance->path_speed = 0;
	instance->path_scale = 1;
	instance->path_orientation = 0;
	instance->path_endaction = 0;
	instance->timeline_index = -1;
	instance->timeline_running = false;
	instance->timeline_speed = 1;
	instance->timeline_position = 0;
	instance->timeline_loop = false;
	instance->bboxIsStale = true;
	return true;
}


Instance* InstanceList::Iterator::Next() {
	if (_byId) {
		unsigned int endpos;
		Instance* ret = _list->GetInstanceByNumber(_id, _pos, &endpos);
		if ((ret) && (endpos >= _limit)) return NULL;
		endpos++;
		_pos = endpos;
		return ret;
	}
	else {
		Instance* ret;
		while (true) {
			if (_pos >= _limit) return NULL;
			ret = (*_list)[_pos];
			_pos++;
			if (ret->exists) break;
		}
		return ret;
	}
}