/*
  ==============================================================================

    DroneManager.cpp
    Created: 19 Oct 2017 7:33:19pm
    Author:  Ben

  ==============================================================================
*/

#include "DroneManager.h"

juce_ImplementSingleton(DroneManager)

DroneManager::DroneManager() :
	BaseManager("Drones")
{
	isSelectable = true;
	disableAllTrigger = addTrigger("Disable All", "Disable all drones");
	enableAllTrigger = addTrigger("Enable All", "Disable all drones");
	connectAllTrigger = addTrigger("Connect All", "Connect all drones");
	connectAllNotConnectedTrigger = addTrigger("Connect All Not Connected", "Connect all drones that are not connected yet");
	connectSelectedTrigger = addTrigger("Connect Selected", "Connect all selected drones. You can select multiple with Ctrl");
	disableNotFlyingsTrigger = addTrigger("Disable Not Flyings", "Disable all not flying drones");
	recalibrateAll = addTrigger("Recalibrate All", "Reset all Kalman estimations");
	takeOffAll = addTrigger("TakeOff All", "");
	landAll = addTrigger("Land All", "");
	unlockAll = addTrigger("Unlock All", "");

	launchTime = addFloatParameter("Launch Time", "How much time the drone keep the launching mode", 2, 0, 5);
	launchForce = addFloatParameter("Launch Force", "The thrust amount. 1 = 10000, 10 = 100000", 6, 1, 6);
	launchMinForce = addFloatParameter("Launch Min Force", "The min thrust amount. 1 = 10000, 10 = 100000", 4, 1, 6);

	flyingLowBatteryThreshold = addFloatParameter("Fly Low Battery Threshold", "Low battery threshold when flying", 2.9f, 2.8f, 3.7f);
	onGroundLowBatteryThreshold = addFloatParameter("On Ground Battery Threshold", "Low battery threshold when on ground", 3.2f, 2.8f, 3.7f);
	lowBatteryTimeCheck = addFloatParameter("Low Battery Time Check", "Amount of time the voltage has to be below threshold to decalre low battery", 1.0f,0,10);
	
	addChildControllableContainer(&launchCurve);
	launchCurve.showUIInEditor = true;
	launchCurve.addItem(0, 1);
	launchCurve.addItem(1, 0);
	launchCurve.items[0]->setEasing(Easing::BEZIER);
	launchCurve.enableSnap->setValue(false);
}

DroneManager::~DroneManager()
{
}

void DroneManager::onContainerTriggerTriggered(Trigger * t)
{
	
	if (t == connectAllTrigger) for (Drone2 * d : items) d->connectTrigger->trigger();
	if (t == connectAllNotConnectedTrigger) for (Drone2 * d : items) if(d->state->getValueDataAsEnum<Drone2::DroneState>() != Drone2::READY) d->connectTrigger->trigger();
	if (t == recalibrateAll) for (Drone2 * d : items) d->calibrateTrigger->trigger();
	if (t == disableAllTrigger) for (Drone2 * d : items) d->enabled->setValue(false);
	if (t == enableAllTrigger) for (Drone2 * d : items) d->enabled->setValue(true);
	if (t == takeOffAll) for (Drone2 * d : items) d->takeOffTrigger->trigger();
	if (t == landAll) for (Drone2 * d : items) d->landTrigger->trigger();
	if (t == unlockAll) for (Drone2 * d : items) d->unlockTrigger->trigger();
	if (t == connectSelectedTrigger) 
	{
		for (Drone2 * d : items)
		{
			if (d->isSelected)
			{
				d->enabled->setValue(true);
				d->connectTrigger->trigger();
			}
		}
	} else if (t == disableNotFlyingsTrigger)
	{
		for (Drone2 * d : items)
		{
			if (d->state->getValueDataAsEnum<Drone2::DroneState>() != Drone2::FLYING) d->enabled->setValue(false);
		}
	}
}

void DroneManager::onContainerParameterChanged(Parameter * p)
{
	if (p == flyingLowBatteryThreshold) for (auto &d : items) d->voltage->setRange(flyingLowBatteryThreshold->floatValue(), d->voltage->maximumValue);
}

var DroneManager::getJSONData()
{
	var data = BaseManager::getJSONData();
	data.getDynamicObject()->setProperty("launchCurve", launchCurve.getJSONData());
	return data;
}

void DroneManager::loadJSONDataInternal(var data)
{
	BaseManager::loadJSONDataInternal(data);
	launchCurve.loadJSONData(data.getProperty("launchCurve", var()));
}
