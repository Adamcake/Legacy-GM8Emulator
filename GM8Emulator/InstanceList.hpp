#ifndef _A_INSTANCELIST_HPP_
#define _A_INSTANCELIST_HPP_
struct Instance;

// This is like an std::vector of Instance objects. The list will ALWAYS be in order of instance id.
class InstanceList {
	private:
		Instance* _list;
		unsigned int _size;
		unsigned int _capacity;
		unsigned int _highestIdAdded;

	public:
		InstanceList();
		~InstanceList();

		// Adds a new instance and returns the pointer. This should always be used instead of Instance's constructor.
		// The instance's ID will be set to the provided value. All its other values will be undefined.
		Instance* AddInstance(unsigned int id);

		// Delete instance with the given id
		void DeleteInstance(unsigned int id);

		// Remove all instances
		void ClearAll();

		// Remove all non-persistent instances
		void ClearNonPersistent();

		// Get the number of active instances
		inline unsigned int Count() const { return _size; }

		// Get the Instance from the specified position in _list (like with std::vector)
		Instance* operator[](unsigned int index);
};

#endif