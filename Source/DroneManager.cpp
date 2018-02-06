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
	resetAllKalman = addTrigger("Reset All Kalman", "Reset all Kalman estimations");

	flyingLowBatteryThreshold = addFloatParameter("Fly Low Battery Threshold", "Low battery threshold when flying", 2.9f, 2.8f, 3.7f);
	onGroundLowBatteryThreshold = addFloatParameter("On Ground Battery Threshold", "Low battery threshold when on ground", 3.2f, 2.8f, 3.7f);
	lowBatteryTimeCheck = addFloatParameter("Low Battery Time Check", "Amount of time the voltage has to be below threshold to decalre low battery", 1.0f,0,10);


}

DroneManager::~DroneManager()
{
}

void DroneManager::onContainerTriggerTriggered(Trigger * t)
{
	if (t == connectAllTrigger) for (Drone * d : items) d->connectTrigger->trigger();
	if (t == connectAllNotConnectedTrigger) for (Drone * d : items) if(d->droneState->getValueDataAsEnum<Drone::DroneState>() != Drone::READY) d->connectTrigger->trigger();
	if (t == resetAllKalman) for (Drone * d : items) d->resetKalmanTrigger->trigger();
	if (t == disableAllTrigger) for (Drone * d : items) d->enabled->setValue(false);
	if (t == enableAllTrigger) for (Drone * d : items) d->enabled->setValue(true);
	if (t == connectSelectedTrigger) 
	{
		for (Drone * d : items)
		{
			if (d->isSelected)
			{
				d->enabled->setValue(true);
				d->connectTrigger->trigger();
			}
		}
	} else if (t == disableNotFlyingsTrigger)
	{
		for (Drone * d : items)
		{
			if (!d->isFlying->boolValue()) d->enabled->setValue(false);
		}
	}

}

void DroneManager::onContainerParameterChanged(Parameter * p)
{
	if (p == flyingLowBatteryThreshold) for (auto &d : items) d->voltage->setRange(flyingLowBatteryThreshold->floatValue(), d->voltage->maximumValue);
}
