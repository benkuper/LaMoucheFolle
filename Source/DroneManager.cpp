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
	connectAllTrigger = addTrigger("Connect All", "Connect all drones");
	connectAllNotConnectedTrigger = addTrigger("Connect All Not Connected", "Connect all drones that are not connected yet");
	resetAllKalman = addTrigger("Reset All Kalman", "Reset all Kalman estimations");

	flyingLowBatteryThreshold = addFloatParameter("Fly Low Battery Threshold", "Low battery threshold when flying", 2.9f, 2.8f, 3.7f);
	onGroundLowBatteryThreshold = addFloatParameter("On Ground Battery Threshold", "Low battery threshold when on ground", 3.2f, 2.8f, 3.7f);
}

DroneManager::~DroneManager()
{
}

void DroneManager::onContainerTriggerTriggered(Trigger * t)
{
	if (t == connectAllTrigger) for (Drone * d : items) d->connectTrigger->trigger();
	if (t == connectAllNotConnectedTrigger) for (Drone * d : items) if(d->droneState->getValueDataAsEnum<Drone::DroneState>() != Drone::READY) d->connectTrigger->trigger();
	if (t == resetAllKalman) for (Drone * d : items) d->resetKalmanTrigger->trigger();
}
