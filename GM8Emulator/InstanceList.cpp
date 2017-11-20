#include "InstanceList.hpp"
#include "Instance.hpp"
#include <cstring>
#define START_CAPACITY 1

InstanceList::InstanceList() {
	_list = new Instance[START_CAPACITY];
	_size = 0;
	_capacity = START_CAPACITY;
	_highestIdAdded = 0;
}

InstanceList::~InstanceList() {
	delete _list;
}

Instance* InstanceList::AddInstance(unsigned int id) {
	if (_size >= _capacity) {
		Instance* newList = new Instance[_capacity * 2];
		memcpy(newList, _list, _size * sizeof(Instance));
		delete _list;
		_list = newList;
		_capacity *= 2;
	}
	if (_highestIdAdded > id) {
		for (unsigned int i = 0; i < _size; i++) {
			if (_list[i].id < id) {
				memmove(_list + i + 1, _list + i, sizeof(Instance) * (_size - i));
				_size++;
				Instance* ret = _list + i;
				ret->id = id;
				return ret;
			}
		}
	}
	Instance* ret = _list + _size;
	_highestIdAdded = id;
	ret->id = id;
	_size++;
	return ret;
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
		if (_list[i].persistent && (placed != i)) {
			memcpy(_list + placed, _list + i, sizeof(Instance));
			placed++;
		}
	}
	_size = placed;
}

Instance * InstanceList::operator[](unsigned int index) {
	return _list + index;
}
