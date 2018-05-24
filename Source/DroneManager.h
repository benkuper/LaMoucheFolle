/*
  ==============================================================================

    DroneManager.h
    Created: 19 Oct 2017 7:33:19pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Drone2.h"

class DroneManager :
	public BaseManager<Drone2>
{
public:
	juce_DeclareSingleton(DroneManager,true)

	DroneManager();
	~DroneManager();

	FloatParameter * flyingLowBatteryThreshold;
	FloatParameter * onGroundLowBatteryThreshold;
	FloatParameter * lowBatteryTimeCheck;
	FloatParameter * lowBatteryLandTime;
	FloatParameter * lowBatteryLandForce;

	Trigger * connectAllTrigger;
	Trigger * disableAllTrigger;
	Trigger * enableAllTrigger;
	Trigger * connectAllNotConnectedTrigger;
	Trigger * connectSelectedTrigger;
	Trigger * disableNotFlyingsTrigger;
	Trigger * recalibrateAll;
	Trigger * takeOffAll;
	Trigger * landAll;
	Trigger * unlockAll;

	FloatParameter * launchTime;
	FloatParameter * launchForce;
	FloatParameter * launchMinForce;

	Automation launchCurve;

	void onContainerTriggerTriggered(Trigger *) override;
	void onContainerParameterChanged(Parameter *) override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneManager)
};