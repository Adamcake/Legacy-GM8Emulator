#ifndef _A_INSTANCELIST_HPP_
#define _A_INSTANCELIST_HPP_
struct Instance;
class AssetManager;

// This is like an std::vector of Instance objects. The list will ALWAYS be in order of instance id.
class InstanceList {
	private:
		Instance* _list;
		AssetManager* _assetManager;
		unsigned int _size;
		unsigned int _capacity;
		unsigned int _highestIdAdded;

		// Give an Instance its default values - returns false if the Object does not exist and game should close
		bool _InitInstance(Instance* instance, unsigned int id, double x, double y, unsigned int objectId);

	public:
		InstanceList(AssetManager* manager);
		~InstanceList();

		// Adds a new instance and returns the pointer. This should always be used instead of Instance's constructor. This is like instance_create() in GML.
		// The instance's ID, x and y will be set appropriately. All other values will be default for that object type or global default where applicable.
		// If this returns a null pointer, there was an error and the game should close.
		Instance* AddInstance(unsigned int id, double x, double y, unsigned int objectId);

		// Delete instance with the given id
		void DeleteInstance(unsigned int id);

		// Remove all instances
		void ClearAll();

		// Remove all non-persistent instances (also removes deleted instances)
		void ClearNonPersistent();

		// Remove all instances that no longer exist
		void ClearDeleted();

		// Gets instance by a number. Similar to GML, if the number is > 100000 it'll be treated as an instance ID, otherwise an object ID.
		Instance* GetInstanceByNumber(unsigned int id);

		// Get the number of active instances
		inline unsigned int Count() const { return _size; }

		// Get the Instance from the specified position in _list (like with std::vector)
		Instance* operator[](unsigned int index);
};

#endif