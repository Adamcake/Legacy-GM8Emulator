#pragma once

#include <functional>

struct GMLType;
struct Instance;

typedef unsigned int InstanceID;
typedef unsigned int InstanceHandle;

// This is like an std::vector of Instance objects. The list will ALWAYS be in order of instance id.
namespace InstanceList {
    void Init();
    void Finalize();

    // Adds a new instance and returns the pointer. This should always be used instead of Instance's constructor. This is like instance_create() in GML.
    // The instance's ID, x and y will be set appropriately. All other values will be default for that object type or global default where applicable.
    // If this returns a null pointer, there was an error and the game should close.
    InstanceHandle AddInstance(unsigned int id, double x, double y, unsigned int objectId);

    // As above, but using a dynamic instance ID
    InstanceHandle AddInstance(double x, double y, unsigned int objectId);

    // Restore list of instances
    InstanceHandle AddInstances(std::vector<Instance>& instances);

    // Used to inform InstanceList that an instance has changed object type
    void HandleChangedInstance(InstanceHandle handle, unsigned int oldObject, unsigned int newObject);

    // Remove all instances
    void ClearAll();

    // Remove all non-persistent instances (also removes deleted instances)
    void ClearNonPersistent();

    // Remove all instances that no longer exist
    void ClearDeleted();

    // Gets instance by a number. Similar to GML, if the number is > 100000 it'll be treated as an instance ID, otherwise an object ID.
    Instance* GetInstanceByNumber(unsigned int id, size_t startPos = 0, size_t* endPos = nullptr);

    // Gets a dummy instance for use in room creation code.
    // Doesn't need to be destroyed, won't ever be iterated, and doesn't count towards instance_count.
    InstanceHandle GetDummyInstance();

    // Get the number of active instances
    size_t Count();

    // Set the next instance ID to assign after all the static instances are loaded
    void SetLastInstanceID(unsigned int i);

    // Get instance reference from InstanceHandle
    // Note: Instance references should NEVER be stored, as the underlying buffer may be reallocated at any time
    Instance& GetInstance(InstanceHandle);

    // Getters and setters for instance fields
    GMLType* GetField(InstanceHandle instance, uint32_t field);
    void SetField(InstanceHandle instance, uint32_t field, const GMLType& value);
    GMLType* GetField(InstanceHandle instance, uint32_t field, uint32_t array);
    void SetField(InstanceHandle instance, uint32_t field, uint32_t array, const GMLType& value);
    GMLType* GetField(InstanceHandle instance, uint32_t field, uint32_t array1, uint32_t array2);
    void SetField(InstanceHandle instance, uint32_t field, uint32_t array1, uint32_t array2, const GMLType& value);

    // Iterator class for looping over instances. Has two modes of operation: all instances, or all matching a certain object/instance number.
    // Mode of operation is determined by which constructor is used. In other words, pass an ID if you want to iterate by that ID. Otherwise it will iterate all.
    class Iterator {
      private:
        bool _byId;
        unsigned int _id;
        size_t _pos;
        size_t _limit;

      public:
        Iterator() : _pos(0), _byId(false), _limit(InstanceList::Count()) {}
        Iterator(unsigned int id) : _pos(0), _id(id), _byId(true), _limit(InstanceList::Count()) {}
        Iterator(unsigned int id, InstanceHandle startPos);
        InstanceHandle Next();
    };

    class LambdaIterator {
      private:
        std::function<bool(Instance& inst)> func;
        size_t _pos;
        size_t _limit;

      public:
        LambdaIterator(std::function<bool(Instance& inst)> p) : func(p), _pos(0), _limit(InstanceList::Count()) {}
        InstanceHandle Next();
    };

    extern unsigned int NoInstance;
};
