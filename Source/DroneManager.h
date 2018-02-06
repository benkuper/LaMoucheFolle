/*
  ==============================================================================

    DroneManager.h
    Created: 19 Oct 2017 7:33:19pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Drone.h"

class DroneManager :
	public BaseManager<Drone>
{
public:
	juce_DeclareSingleton(DroneManager,true)

	DroneManager();
	~DroneManager();

	FloatParameter * flyingLowBatteryThreshold;
	FloatParameter * onGroundLowBatteryThreshold;
	FloatParameter * lowBatteryTimeCheck;

	Trigger * connectAllTrigger;
	Trigger * disableAllTrigger;
	Trigger * enableAllTrigger;
	Trigger * connectAllNotConnectedTrigger;
	Trigger * connectSelectedTrigger;
	Trigger * disableNotFlyingsTrigger;
	Trigger * resetAllKalman;

	void onContainerTriggerTriggered(Trigger *) override;
	void onContainerParameterChanged(Parameter *) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneManager)
};