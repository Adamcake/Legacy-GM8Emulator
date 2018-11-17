#ifndef _A_INSTANCELIST_HPP_
#define _A_INSTANCELIST_HPP_
#include <pch.h>
struct GMLType;
struct Instance;
typedef unsigned int InstanceID;

// This is like an std::vector of Instance objects. The list will ALWAYS be in order of instance id.
namespace InstanceList {
    void Init();
    void Finalize();

    // Adds a new instance and returns the pointer. This should always be used instead of Instance's constructor. This is like instance_create() in GML.
    // The instance's ID, x and y will be set appropriately. All other values will be default for that object type or global default where applicable.
    // If this returns a null pointer, there was an error and the game should close.
    Instance* AddInstance(unsigned int id, double x, double y, unsigned int objectId);

    // As above, but using a dynamic instance ID
    Instance* AddInstance(double x, double y, unsigned int objectId);

    // Delete instance with the given id
    void DeleteInstance(unsigned int id);

    // Remove all instances
    void ClearAll();

    // Remove all non-persistent instances (also removes deleted instances)
    void ClearNonPersistent();

    // Remove all instances that no longer exist
    void ClearDeleted();

    // Gets instance by a number. Similar to GML, if the number is > 100000 it'll be treated as an instance ID, otherwise an object ID.
    Instance* GetInstanceByNumber(unsigned int id, unsigned int startPos = 0, unsigned int* endPos = nullptr);

    // Gets a dummy instance for use in room creation code.
    // Doesn't need to be destroyed, won't ever be iterated, and doesn't count towards instance_count.
    Instance* GetDummyInstance();
    
    // Get the number of active instances
    unsigned int Count();

    // Set the next instance ID to assign after all the static instances are loaded
    void SetLastInstanceID(unsigned int i);// { _nextInstanceID = i; }

    // Getters and setters for instance fields
    GMLType* GetField(InstanceID instance, unsigned int field);
    void SetField(InstanceID instance, unsigned int field, const GMLType* value);
    GMLType* GetField(InstanceID instance, unsigned int field, unsigned int array);
    void SetField(InstanceID instance, unsigned int field, unsigned int array, const GMLType* value);
    GMLType* GetField(InstanceID instance, unsigned int field, unsigned int array1, unsigned int array2);
    void SetField(InstanceID instance, unsigned int field, unsigned int array1, unsigned int array2, const GMLType* value);

    // Iterator class for looping over instances. Has two modes of operation: all instances, or all matching a certain object/instance number.
    // Mode of operation is determined by which constructor is used. In other words, pass an ID if you want to iterate by that ID. Otherwise it will iterate all.
    class Iterator {
      private:
        bool _byId;
        unsigned int _pos;
        unsigned int _id;
        unsigned int _limit;

      public:
        Iterator() : _pos(0), _byId(false), _limit(InstanceList::Count()) {}
        Iterator(unsigned int id) : _pos(0), _id(id), _byId(true), _limit(InstanceList::Count()) {}
        Iterator(unsigned int id, Instance* start);
        Instance* Next();
    };
};

#endif