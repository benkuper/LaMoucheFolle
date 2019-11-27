/*
  ==============================================================================

    DroneManager.cpp
    Created: 25 Nov 2019 3:33:43pm
    Author:  bkupe

  ==============================================================================
*/

#include "DroneManager.h"

juce_ImplementSingleton(DroneManager)

DroneManager::DroneManager() :
	BaseManager("Drones")
{
	itemDataType = "Drone";

	thumbSize = addIntParameter("Thumb Size", "Size of each item", 128, 32, 256);
	takeOffAll = addTrigger("Take off All", "Take off all ready-to-fly drones");
	landAll = addTrigger("Land All", "Land all flying drones");
}

DroneManager::~DroneManager()
{
}

void DroneManager::addItemInternal(Drone* d, var data)
{
	if(data.isVoid()) d->id->setValue(getFirstAvailableID(d));
}

void DroneManager::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	Drone * d = ControllableUtil::findParentAs<Drone>(c);
	if (d != nullptr)
	{
		if (c == d->id)
		{
			Drone* dd = getDroneWithID(d->id->intValue(), d);
			if (dd != nullptr) dd->id->setValue(getFirstAvailableID());
		}
	}
}

int DroneManager::getFirstAvailableID(Drone* exclude)
{
	int result = 0;
	Drone* d = getDroneWithID(result);
	while (d != nullptr && d != exclude)
	{
		result++;
		d = getDroneWithID(result);
	}

	return result;
}

Drone* DroneManager::getDroneWithID(int id, Drone * exclude)
{
	for (auto& i : items)
	{
		if (i == exclude) continue;
		if (i->id->intValue() == id) return i;
	}
	return nullptr;
}
