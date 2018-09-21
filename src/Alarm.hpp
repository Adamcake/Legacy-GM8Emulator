#ifndef _A_ALARM_HPP_
#define _A_ALARM_HPP_
#include <pch.h>
#include <map>

typedef unsigned int InstanceID;

void AlarmSet(InstanceID instance, unsigned int alarm, int value);
int AlarmGet(InstanceID instance, int alarm);
void AlarmUpdateAll();
void AlarmDelete(InstanceID instance, unsigned int alarm);
const std::map<unsigned int, int> AlarmGetMap(InstanceID instance);
void AlarmRemoveInstance(InstanceID instance);

#endif