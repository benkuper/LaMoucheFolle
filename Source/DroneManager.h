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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneManager)
};