/*
  ==============================================================================

    DroneManager.h
    Created: 25 Nov 2019 3:33:43pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "Drone.h"

class DroneManager :
	public BaseManager<Drone>
{
public:
	juce_DeclareSingleton(DroneManager, true)
	DroneManager();
	~DroneManager();

	IntParameter * thumbSize;
	Trigger* takeOffAll;
	Trigger* landAll;

	void addItemInternal(Drone* d, var data) override;

	void onContainerTriggerTriggered(Trigger * t) override;
	void onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c) override;
	int getFirstAvailableID(Drone* exclude = nullptr);
	Drone* getDroneWithID(int id, Drone * exclude = nullptr);
};