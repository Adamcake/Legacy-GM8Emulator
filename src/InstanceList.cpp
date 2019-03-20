#include "InstanceList.hpp"
#include "AssetManager.hpp"
#include "CRGMLType.hpp"
#include "CodeActionManager.hpp"
#include "Instance.hpp"
#include "Renderer.hpp"
#include "Tile.hpp"
#include <algorithm>  // for remove_if
#include <vector>


// Abstract type that InstanceList can pool and draw. Must be drawable
struct PooledType {
    bool used = false;
    virtual bool Draw() = 0;
    virtual int GetDepth() = 0;
    virtual int GetObjectIndex() = 0;
};

struct PooledInstance : public PooledType {
    Instance instance;
    bool Draw();
    int GetDepth() {return instance.depth;}
    int GetObjectIndex() {return instance.object_index;}
};

struct PooledTile : public PooledType {
    Tile tile;
    bool Draw();
    int GetDepth() {return tile.depth;}
    int GetObjectIndex() {return -1;}
};

// Template class for creating memory pools
template <class T> struct Pool {
    bool used;
    size_t size;
    T* data;
    Pool(size_t pSize) : size(pSize), used(true) {
        data = new T[pSize];
    }
    Pool(const Pool& other) = delete;
    Pool(Pool&& other) : data(other.data), size(other.size), used(other.used) {
        other.data = nullptr;
    }
    ~Pool() {
        delete[] data;
    }
    Pool& operator=(const Pool& other) = delete;
    Pool& operator=(Pool&& other) {
        data = other.data;
        size = other.size;
        used = other.used;
        other.data = nullptr;
        return *this;
    }
};
std::vector<Pool<PooledInstance>> _instancePools;
std::vector<Pool<PooledTile>> _tilePools;
size_t _largestPoolSize;

std::vector<PooledInstance*> _iterationOrder;
std::vector<PooledTile*> _tiles;
std::vector<PooledType*> _drawOrder;

Pool<PooledInstance>& _addInstancePool(size_t size) {
    size_t poolCount = _instancePools.size();
    _instancePools.push_back(Pool<PooledInstance>(size));
    return _instancePools[poolCount];
}

Pool<PooledTile>& _addTilePool(size_t size) {
    size_t poolCount = _tilePools.size();
    _tilePools.push_back(Pool<PooledTile>(size));
    return _tilePools[poolCount];
}

// Last dynamic instance ID and tile ID to be assigned
unsigned int _lastInstanceID;
unsigned int _lastTileID;

// Give an Instance its default values - returns false if the Object does not exist and game should close
bool _InitInstance(Instance* instance, unsigned int id, double x, double y, unsigned int objectId);

void InstanceList::Init() {
    _largestPoolSize = 1024;
    _addInstancePool(_largestPoolSize);
    _iterationOrder.reserve(1024);
    _drawOrder.reserve(1024);
}

void InstanceList::Finalize() {
    _instancePools.clear();
    _iterationOrder.clear();
    _drawOrder.clear();
}

InstanceHandle InstanceList::AddInstance(InstanceID id, double x, double y, unsigned int objectId) {
    PooledInstance* place = nullptr;
    // Iterate all our pools looking for an unused spot
    for (Pool<PooledInstance>& pool : _instancePools) {
        if (pool.used) {
            for (unsigned int i = 0; i < pool.size; i++) {
                PooledInstance& pooledInst = pool.data[i];
                if (!pooledInst.used) {
                    pooledInst.used = true;
                    place = &pooledInst;
                    break;
                }
            }
            if (place != nullptr) {
                // break out of outer loop if we've already placed the instance
                break;
            }
        }
    }
    // If we didn't manage to place the instance yet, we need to add a new pool
    if (place == nullptr) {
        // Mark all our pre-existing pools as disused (won't be allocated into, and will be de-allocated when empty)
        for (Pool<PooledInstance>& pool : _instancePools) {
            pool.used = false;
        }
        // Make the new pool
        _largestPoolSize *= 2;
        Pool<PooledInstance>& pool = _addInstancePool(_largestPoolSize);

        // Use the first element in the instance pool as our new instance
        pool.data[0].used = true;
        place = &pool.data[0];
    }

    InstanceHandle ret = static_cast<InstanceHandle>(_iterationOrder.size());
    _iterationOrder.push_back(place);
    _drawOrder.push_back(place);
    if (_InitInstance(&place->instance, id, x, y, objectId)) {
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

unsigned int InstanceList::AddTile(unsigned int id, int background, int left, int top, unsigned int width, unsigned int height, double x, double y, int depth) {
    PooledTile* place = nullptr;
    // Iterate all our pools looking for an unused spot
    for (Pool<PooledTile>& pool : _tilePools) {
        if (pool.used) {
            for (unsigned int i = 0; i < pool.size; i++) {
                PooledTile& pooledTile = pool.data[i];
                if (!pooledTile.used) {
                    pooledTile.used = true;
                    place = &pooledTile;
                    break;
                }
            }
            if (place != nullptr) {
                // break out of outer loop if we've already placed the instance
                break;
            }
        }
    }
    // If we didn't manage to place the instance yet, we need to add a new pool
    if (place == nullptr) {
        // Mark all our pre-existing pools as disused (won't be allocated into, and will be de-allocated when empty)
        for (Pool<PooledTile>& pool : _tilePools) {
            pool.used = false;
        }
        // Make the new pool
        _largestPoolSize *= 2;
        Pool<PooledTile>& pool = _addTilePool(_largestPoolSize);

        // Use the first element in the instance pool as our new instance
        pool.data[0].used = true;
        place = &pool.data[0];
    }

    _tiles.push_back(place);
    _drawOrder.push_back(place);
    place->tile = Tile(x, y, background, left, top, width, height, depth, id);
    return id;
}

unsigned int InstanceList::AddTile(int background, int left, int top, unsigned int width, unsigned int height, double x, double y, int depth) {
    _lastTileID++;
    return AddTile(_lastTileID, background, left, top, width, height, x, y, depth);
}

void InstanceList::AddInstances(const std::vector<Instance>& instances) {
    if (!instances.size()) return;
    unsigned int pos = 0;
    // Go through all used pools looking for places to put these instances
    for (Pool<PooledInstance>& pool : _instancePools) {
        if (pool.used) {
            for (unsigned int i = 0; i < pool.size; i++) {
                PooledInstance& pooledInst = pool.data[i];
                if (!pooledInst.used) {
                    pooledInst.instance = instances[pos];
                    pooledInst.used = true;
                    _iterationOrder.push_back(&pooledInst);
                    _drawOrder.push_back(&pooledInst);
                    pos++;
                    if (pos == instances.size()) return;
                }
            }
        }
    }
    // Some instances weren't placed in our pool, so we need a new pool for them
    size_t remainingCount = instances.size() - pos;
    while (_largestPoolSize < remainingCount) _largestPoolSize *= 2;
    Pool<PooledInstance>& newPool = _addInstancePool(_largestPoolSize);

    unsigned int poolPos = 0;
    while (pos < instances.size()) {
        newPool.data[poolPos].used = true;
        newPool.data[poolPos].instance = instances[pos];
        poolPos++;
        pos++;
    }
}

void InstanceList::ClearAll() {
    for (PooledInstance* inst : _iterationOrder) {
        inst->used = false;
    }
    for (PooledTile* tile : _tiles) {
        tile->used = false;
    }
    auto it = std::remove_if(_instancePools.begin(), _instancePools.end(), [](Pool<PooledInstance>& pool) { return !pool.used; });
    _instancePools.erase(it, _instancePools.end());
    _iterationOrder.clear();
    _drawOrder.clear();
    _tiles.clear();
}

void InstanceList::ClearNonPersistent() {
    for (const Pool<PooledInstance>& pool : _instancePools) {
        for (unsigned int i = 0; i < pool.size; i++) {
            PooledInstance& pooledInst = pool.data[i];
            if (pooledInst.used) {
                if ((!pooledInst.instance.persistent) || (!pooledInst.instance.exists)) pooledInst.used = false;
            }
        }
    }
    for (PooledTile* tile : _tiles) {
        tile->used = false;
    }
    auto it = std::remove_if(_iterationOrder.begin(), _iterationOrder.end(), [](PooledInstance* inst) { return !inst->used; });
    _iterationOrder.erase(it, _iterationOrder.end());
    auto it2 = std::remove_if(_drawOrder.begin(), _drawOrder.end(), [](PooledType* inst) { return !inst->used; });
    _drawOrder.erase(it2, _drawOrder.end());
    _tiles.clear();
}

void InstanceList::ClearDeleted() {
    for (const Pool<PooledInstance>& pool : _instancePools) {
        for (unsigned int i = 0; i < pool.size; i++) {
            PooledInstance& pooledInst = pool.data[i];
            if (pooledInst.used) {
                if (!pooledInst.instance.exists) pooledInst.used = false;
            }
        }
    }
    auto it = std::remove_if(_iterationOrder.begin(), _iterationOrder.end(), [](PooledInstance* inst) { return !inst->used; });
    _iterationOrder.erase(it, _iterationOrder.end());
    auto it2 = std::remove_if(_drawOrder.begin(), _drawOrder.end(), [](PooledType* inst) { return !inst->used; });
    _drawOrder.erase(it2, _drawOrder.end());
}

bool InstanceList::DrawEverything() {
    std::sort(_drawOrder.begin(), _drawOrder.end(), [](PooledType*& l, PooledType*& r) {
        return (l->GetDepth() == r->GetDepth()) ? (l->GetObjectIndex() > r->GetObjectIndex()) : (l->GetDepth() > r->GetDepth());
    });
    for (PooledType*& toDraw : _drawOrder) {
        if (!toDraw->Draw()) return false;
    }
    return true;
}

Instance* InstanceList::GetInstanceByNumber(unsigned int num, size_t startPos, size_t* endPos) {
    if (num > 100000) {
        // Instance ID
        for (auto i = _iterationOrder.begin() + startPos; i != _iterationOrder.end(); i++) {
            if ((*i)->instance.id == num) {
                if (endPos) (*endPos) = startPos;
                return ((*i)->instance.exists) ? &(*i)->instance : nullptr;
            }
            startPos++;
        }
    }
    else {
        // Object ID
        for (auto i = _iterationOrder.begin() + startPos; i != _iterationOrder.end(); i++) {
            Object* o = AssetManager::GetObject((*i)->instance.object_index);
            if (o->identities.count(num) && (*i)->instance.exists) {
                if (endPos) (*endPos) = startPos;
                return &(*i)->instance;
            }
            startPos++;
        }
    }
    if (endPos) (*endPos) = startPos;
    return nullptr;
}

Instance _dummy;
Instance& InstanceList::GetInstance(InstanceHandle handle) {
    if (handle == DummyInstance) return _dummy;
    return _iterationOrder[handle]->instance;
}

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

    return DummyInstance;
}

size_t InstanceList::Count() {
    return _iterationOrder.size();
}

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
uint32_t InstanceList::DummyInstance = static_cast<uint32_t>(-2);

InstanceList::Iterator::Iterator(unsigned int id, InstanceHandle startPos) : _pos(startPos), _id(id), _byId(true), _limit(InstanceList::Count()) {}

InstanceHandle InstanceList::Iterator::Next() {
    if (_byId) {
        size_t endpos;
        Instance* ret = InstanceList::GetInstanceByNumber(_id, _pos, &endpos);
        if (endpos >= _limit) return NoInstance;
        _pos = endpos + 1;
        return static_cast<InstanceHandle>(endpos);
    }
    else {
        InstanceHandle ret;
        while (true) {
            if (_pos >= _limit) return NoInstance;
            ret = static_cast<InstanceHandle>(_pos);
            _pos++;
            if (_iterationOrder[ret]->instance.exists) break;
        }
        return ret;
    }
}

void InstanceList::SetLastIDs(unsigned int instance, unsigned int tile) {
    _lastInstanceID = instance;
    _lastTileID = tile;
}

GMLType* InstanceList::GetField(InstanceHandle instance, uint32_t field) {
    return &GetInstance(instance)._fields[field][0];
}
void InstanceList::SetField(InstanceHandle instance, uint32_t field, const GMLType& value) {
    GetInstance(instance)._fields[field][0] = value;
}
GMLType* InstanceList::GetField(InstanceHandle instance, uint32_t field, uint32_t array) {
    return &GetInstance(instance)._fields[field][array];
}
void InstanceList::SetField(InstanceHandle instance, uint32_t field, uint32_t array, const GMLType& value) {
    GetInstance(instance)._fields[field][array] = value;
}
GMLType* InstanceList::GetField(InstanceHandle instance, uint32_t field, uint32_t array1, uint32_t array2) {
    return &GetInstance(instance)._fields[field][(array1 * 32000) + array2];
}
void InstanceList::SetField(InstanceHandle instance, uint32_t field, uint32_t array1, uint32_t array2, const GMLType& value) {
    GetInstance(instance)._fields[field][(array1 * 32000) + array2] = value;
}

InstanceHandle InstanceList::LambdaIterator::Next() {
    while (_pos < _limit) {
        if (_iterationOrder[_pos]->instance.exists) {
            if (func(_iterationOrder[_pos]->instance)) {
                _pos++;
                return static_cast<InstanceHandle>(_pos - 1);
            }
        }
        _pos++;
    }
    return NoInstance;
}


bool PooledInstance::Draw() {
    // I'm sorry in advance for whoever maintains this, it's not my fault, gamemaker is stupid and it's the only way
    if (instance.visible) {
        Object* obj = AssetManager::GetObject(instance.object_index);
        if (obj->events[8].count(0)) {
            // This object has a custom draw event.
            // TODO: Make this not O(n) please
            if (!CodeActionManager::RunInstanceEvent(8, 0, InstanceList::Iterator(instance.id).Next(), InstanceList::NoInstance, instance.object_index)) return false;
        }
        else {
            // This is the default draw action if no draw event is present for this object.
            if (instance.sprite_index >= 0) {
                Sprite* sprite = AssetManager::GetSprite(instance.sprite_index);
                if (sprite->exists) {
                    RDrawImage(sprite->frames[static_cast<int>(instance.image_index) % sprite->frameCount], instance.x, instance.y, instance.image_xscale, instance.image_yscale, instance.image_angle,
                        instance.image_blend, instance.image_alpha);
                }
                else {
                    // Tried to draw non-existent sprite
                    return false;
                }
            }
        }
    }
    return true;
}

bool PooledTile::Draw() {
    if (tile.backgroundIndex < 0) return false;
    Background* bg = AssetManager::GetBackground(tile.backgroundIndex);
    if (!bg->exists) return false;
    RDrawPartialImage(bg->image, tile.x, tile.y, 1, 1, 0, 0xFFFFFFFF, 1, tile.tileX, tile.tileY, tile.width, tile.height);
    return true;
}