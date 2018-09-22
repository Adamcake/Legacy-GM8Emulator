#include "Alarm.hpp"

std::map<InstanceID, std::map<unsigned int, int>> _alarms;

void AlarmSet(InstanceID instance, unsigned int alarm, int value) {
	_alarms[instance][alarm] = value;
}

int AlarmGet(InstanceID instance, int alarm) {
	return _alarms[instance][alarm];
}

void AlarmUpdateAll() {
	for (const auto& i : _alarms) {
		for (const auto& a : i.second) {
			if (_alarms[i.first][a.first] > 0) {
				_alarms[i.first][a.first]--;
			}
		}
	}
}

void AlarmDelete(InstanceID instance, unsigned int alarm) {
	_alarms[instance].erase(alarm);
	if (_alarms[instance].size() == 0) _alarms.erase(instance);
}

const std::map<unsigned int, int> AlarmGetMap(InstanceID instance) {
	return _alarms[instance];
}

void AlarmRemoveInstance(InstanceID instance) {
	_alarms.erase(instance);
}