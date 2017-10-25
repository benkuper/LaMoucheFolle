/*
  ==============================================================================

    DroneManagerUI.h
    Created: 24 Oct 2017 11:19:35am
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "DroneManager.h"
#include "DroneUI.h"

class DroneManagerUI :
	public BaseManagerShapeShifterUI<DroneManager, Drone, DroneUI>
{
public:
	DroneManagerUI(const String &name, DroneManager * manager);
	~DroneManagerUI();

	static DroneManagerUI * create(const String &name) { return new DroneManagerUI(name, DroneManager::getInstance()); }
};