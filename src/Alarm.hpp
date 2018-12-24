#pragma once

#include <map>

typedef unsigned int InstanceID;

void AlarmSet(InstanceID instance, unsigned int alarm, int value);
int AlarmGet(InstanceID instance, int alarm);
void AlarmUpdateAll();
void AlarmDelete(InstanceID instance, unsigned int alarm);
void AlarmDeleteAll();
std::map<unsigned int, int>& AlarmGetMap(InstanceID instance);
void AlarmRemoveInstance(InstanceID instance);
