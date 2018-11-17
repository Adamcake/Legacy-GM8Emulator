#include <pch.h>
#include "InstanceList.hpp"
#include "Alarm.hpp"
#include "AssetManager.hpp"
#include "CRGMLType.hpp"
#include "Instance.hpp"
#define INSTANCE_CAPACITY 65536

// Field/array map (if you're maintaining this: god help you)
std::map<InstanceID, std::map<unsigned int, std::map<int, GMLType>>> _arrays;

Instance* _list;
unsigned int _size;
unsigned int _highestIdAdded;

// Last dynamic instance ID to be assigned
unsigned int _lastInstanceID;

// Give an Instance its default values - returns false if the Object does not exist and game should close
bool _InitInstance(Instance* instance, unsigned int id, double x, double y, unsigned int objectId);

void _handleDeletedInstance(InstanceID instance) {
    AlarmRemoveInstance(instance);
    _arrays.erase(instance);
}


void InstanceList::Init() {
    _list = new Instance[INSTANCE_CAPACITY];
    _size = 0;
    _highestIdAdded = 0;
}

void InstanceList::Finalize() { delete[] _list; }

Instance* InstanceList::AddInstance(unsigned int id, double x, double y, unsigned int objectId) {
    if (_highestIdAdded > id) {
        for (unsigned int i = 0; i < _size; i++) {
            if (_list[i].id > id) {
                memmove(_list + i + 1, _list + i, sizeof(Instance) * (_size - i));
                _size++;
                Instance* ret = _list + i;
                if (_InitInstance(ret, id, x, y, objectId)) {
                    return ret;
                }
                else
                    return NULL;
            }
        }
    }
    Instance* ret = _list + _size;
    _highestIdAdded = id;
    _size++;
    if (_InitInstance(ret, id, x, y, objectId)) {
        return ret;
    }
    else
        return NULL;
}

Instance* InstanceList::AddInstance(double x, double y, unsigned int objectId) {
    _lastInstanceID++;
    return AddInstance(_lastInstanceID, x, y, objectId);
}

void InstanceList::DeleteInstance(unsigned int id) {
    AlarmRemoveInstance(id);
    for (unsigned int i = 0; i < _size; i++) {
        if (_list[i].id > id) {
            memmove(_list + i, _list + i + 1, sizeof(Instance) * (_size - (i + 1)));
            _size--;
            return;
        }
    }
}

void InstanceList::ClearAll() {
    AlarmDeleteAll();
    _arrays.clear();
    _size = 0;
}

void InstanceList::ClearNonPersistent() {
    unsigned int placed = 0;
    for (unsigned int i = 0; i < _size; i++) {
        if (_list[i].persistent && _list[i].exists) {
            if (placed != i) _list[placed] = _list[i];
            placed++;
        }
        else {
            _handleDeletedInstance(_list[i].id);
        }
    }
    _size = placed;
}

void InstanceList::ClearDeleted() {
    unsigned int placed = 0;
    for (unsigned int i = 0; i < _size; i++) {
        if (_list[i].exists) {
            if (placed != i) _list[placed] = _list[i];
            placed++;
        }
        else {
            _handleDeletedInstance(_list[i].id);
        }
    }
    _size = placed;
}

Instance* InstanceList::GetInstanceByNumber(unsigned int num, unsigned int startPos, unsigned int* endPos) {
    if (num > 100000) {
        // Instance ID
        for (unsigned int i = startPos; i < _size; i++) {
            if (_list[i].id == num) {
                if (endPos) (*endPos) = i;
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
            Object* o = AssetManager::GetObject(_list[i].object_index);
            if (o->identities.count(num) && _list[i].exists) {
                if (endPos) (*endPos) = i;
                return _list + i;
            }
        }
    }
    return NULL;
}

Instance _dummy;
Instance* InstanceList::GetDummyInstance() {
    AlarmRemoveInstance(0);
    _arrays.erase(0);
    _dummy.id = 0;
    _dummy.object_index = 0;
    _dummy.solid = false;
    _dummy.visible = true;
    _dummy.persistent = false;
    _dummy.depth = 0;
    _dummy.sprite_index = -1;
    _dummy.image_alpha = 1;
    _dummy.image_blend = 0xFFFFFF;
    _dummy.image_index = 0;
    _dummy.image_speed = 1;
    _dummy.image_xscale = 1;
    _dummy.image_yscale = 1;
    _dummy.image_angle = 0;
    _dummy.mask_index = -1;
    _dummy.direction = 0;
    _dummy.gravity = 0;
    _dummy.gravity_direction = 270;
    _dummy.hspeed = 0;
    _dummy.vspeed = 0;
    _dummy.speed = 0;
    _dummy.friction = 0;
    _dummy.x = 0.0;
    _dummy.y = 0.0;
    _dummy.xprevious = 0.0;
    _dummy.yprevious = 0.0;
    _dummy.xstart = 0.0;
    _dummy.ystart = 0.0;
    _dummy.path_index = -1;
    _dummy.path_position = 0;
    _dummy.path_positionprevious = 0;
    _dummy.path_speed = 0;
    _dummy.path_scale = 1;
    _dummy.path_orientation = 0;
    _dummy.path_endaction = 0;
    _dummy.timeline_index = -1;
    _dummy.timeline_running = false;
    _dummy.timeline_speed = 1;
    _dummy.timeline_position = 0;
    _dummy.timeline_loop = false;
    _dummy.bbox_bottom = -100000;
    _dummy.bbox_right = -100000;
    _dummy.bbox_left = -100000;
    _dummy.bbox_top = -100000;
    _dummy.bboxIsStale = false;
    return &_dummy;
}

unsigned int InstanceList::Count() { return _size; }


// Private

bool _InitInstance(Instance* instance, unsigned int id, double x, double y, unsigned int objectId) {
    Object* obj = AssetManager::GetObject(objectId);
    if (!obj->exists) return false;
    instance->exists = true;
    instance->id = id;

    instance->object_index = objectId;
    instance->solid = obj->solid;
    instance->visible = obj->visible;
    instance->persistent = obj->persistent;
    instance->depth = obj->depth;
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


InstanceList::Iterator::Iterator(unsigned int id, Instance* start) :
    _pos(static_cast<unsigned int>(start - _list)), _id(id), _byId(true), _limit(InstanceList::Count()) {}

Instance* InstanceList::Iterator::Next() {
    if (_byId) {
        unsigned int endpos;
        Instance* ret = InstanceList::GetInstanceByNumber(_id, _pos, &endpos);
        if (endpos >= _limit) return NULL;
        endpos++;
        _pos = endpos;
        return ret;
    }
    else {
        Instance* ret;
        while (true) {
            if (_pos >= _limit) return NULL;
            ret = &_list[_pos];
            _pos++;
            if (ret->exists) break;
        }
        return ret;
    }
}

void InstanceList::SetLastInstanceID(unsigned int i) {
    _lastInstanceID = i;
}

GMLType* InstanceList::GetField(InstanceID instance, unsigned int field) {
    return &_arrays[instance][field][0];
}
void InstanceList::SetField(InstanceID instance, unsigned int field, const GMLType* value) {
    _arrays[instance][field][0] = *value;
}

GMLType* InstanceList::GetField(InstanceID instance, unsigned int field, unsigned int array) {
    return &_arrays[instance][field][array];
}

void InstanceList::SetField(InstanceID instance, unsigned int field, unsigned int array, const GMLType* value) {
    _arrays[instance][field][array] = *value;
}

GMLType* InstanceList::GetField(InstanceID instance, unsigned int field, unsigned int array1, unsigned int array2) {
    return &_arrays[instance][field][(array1 * 32000) + array2];
}

void InstanceList::SetField(InstanceID instance, unsigned int field, unsigned int array1, unsigned int array2, const GMLType* value) {
    _arrays[instance][field][(array1 * 32000) + array2] = *value;
}