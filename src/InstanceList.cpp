#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "CRGMLType.hpp"
#include "Instance.hpp"
#include <algorithm>
#define INSTANCE_CAPACITY 65536

// Field/array map (if you're maintaining this: god help you)
// std::map<InstanceID, std::map<unsigned int, std::map<int, GMLType>>> _arrays;

std::vector<Instance> _list;

// Last dynamic instance ID to be assigned
unsigned int _lastInstanceID;

// Give an Instance its default values - returns false if the Object does not exist and game should close
bool _InitInstance(Instance* instance, unsigned int id, double x, double y, unsigned int objectId);

void InstanceList::Init() { /*_list.reserve(INSTANCE_CAPACITY);*/ }
void InstanceList::Finalize() { _list.clear(); }

InstanceHandle InstanceList::AddInstance(unsigned int id, double x, double y, unsigned int objectId) {
    InstanceHandle ret = static_cast<InstanceHandle>(_list.size());
    _list.push_back(Instance());
    if (_InitInstance(&_list[ret], id, x, y, objectId)) {
        return ret;
    }
    else {
        return NoInstance;
    }
}

InstanceHandle InstanceList::AddInstance(double x, double y, unsigned int objectId) {
    _lastInstanceID++;
    return AddInstance(_lastInstanceID, x, y, objectId);
}

void InstanceList::ClearAll() {
    _list.clear();
}

void InstanceList::ClearNonPersistent() {
    auto it = std::remove_if(_list.begin(), _list.end(), [](Instance& inst) { return (!inst.persistent) || (!inst.exists); });
    _list.erase(it, _list.end());
}

void InstanceList::ClearDeleted() {
    auto it = std::remove_if(_list.begin(), _list.end(), [](Instance& inst) { return !inst.exists; });
    _list.erase(it, _list.end());
}

Instance* InstanceList::GetInstanceByNumber(unsigned int num, unsigned int startPos, unsigned int* endPos) {
    if (num > 100000) {
        // Instance ID
        for (auto i = _list.begin() + startPos; i != _list.end(); i++) {
            if ((*i).id == num) {
                if (endPos) (*endPos) = startPos;
                return ((*i).exists) ? &(*i) : nullptr;
            }
            startPos++;
        }
    }
    else {
        // Object ID
        for (auto i = _list.begin() + startPos; i != _list.end(); i++) {
            Object* o = AssetManager::GetObject(i->object_index);
            if (o->identities.count(num) && (*i).exists) {
                if (endPos) (*endPos) = startPos;
                return &(*i);
            }
            startPos++;
        }
    }
    if (endPos) (*endPos) = startPos;
    return nullptr;
}

Instance& InstanceList::GetInstance(InstanceHandle handle) { return _list[handle]; }

Instance _dummy;
InstanceHandle InstanceList::GetDummyInstance() {
    _dummy.id = 0;
    _dummy.object_index = 0;
    _dummy.solid = false;
    _dummy.visible = true;
    _dummy.persistent = false;
    _dummy.exists = false;
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
    _dummy._fields.clear();
    _dummy._alarms.clear();

    InstanceHandle ret = static_cast<InstanceHandle>(_list.size());
    _list.push_back(_dummy);
    return ret;
}

size_t InstanceList::Count() { return _list.size(); }

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

    instance->_fields.clear();
    instance->_alarms.clear();
    return true;
}


uint32_t InstanceList::NoInstance = static_cast<uint32_t>(-1);

InstanceList::Iterator::Iterator(unsigned int id, InstanceHandle startPos) : _pos(startPos), _id(id), _byId(true), _limit(InstanceList::Count()) {}

InstanceHandle InstanceList::Iterator::Next() {
    if (_byId) {
        unsigned int endpos;
        Instance* ret = InstanceList::GetInstanceByNumber(_id, _pos, &endpos);
        if (endpos >= _limit) return NoInstance;
        _pos = endpos + 1;
        return endpos;
    }
    else {
        InstanceHandle ret;
        while (true) {
            if (_pos >= _limit) return NoInstance;
            ret = _pos;
            _pos++;
            if (_list[ret].exists) break;
        }
        return ret;
    }
}

void InstanceList::SetLastInstanceID(unsigned int i) { _lastInstanceID = i; }

GMLType* InstanceList::GetField(InstanceHandle instance, uint32_t field) { return &_list[instance]._fields[field][0]; }
void InstanceList::SetField(InstanceHandle instance, uint32_t field, const GMLType* value) { _list[instance]._fields[field][0] = *value; }
GMLType* InstanceList::GetField(InstanceHandle instance, uint32_t field, uint32_t array) { return &_list[instance]._fields[field][array]; }
void InstanceList::SetField(InstanceHandle instance, uint32_t field, uint32_t array, const GMLType* value) { _list[instance]._fields[field][array] = *value; }
GMLType* InstanceList::GetField(InstanceHandle instance, uint32_t field, uint32_t array1, uint32_t array2) { return &_list[instance]._fields[field][(array1 * 32000) + array2]; }

void InstanceList::SetField(InstanceHandle instance, unsigned int field, unsigned int array1, unsigned int array2, const GMLType* value) {
    _list[instance]._fields[field][(array1 * 32000) + array2] = *value;
}

InstanceHandle InstanceList::LambdaIterator::Next() {
    while (_pos < _limit) {
        if (_list[_pos].exists) {
            if (func(_list[_pos])) {
                _pos++;
                return static_cast<InstanceHandle>(_pos);
            }
        }
        _pos++;
    }
    return NoInstance;
}
