/*
  ==============================================================================

    DroneManagerUI.h
    Created: 25 Nov 2019 3:33:57pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../DroneManager.h"
#include "DroneUI.h"

class DroneManagerUI :
	public BaseManagerShapeShifterUI<DroneManager, Drone, DroneUI>
{
public :
	DroneManagerUI(const String& name, DroneManager* manager);
	~DroneManagerUI();
	
	static DroneManagerUI* create(const String& name) { return new DroneManagerUI(name, DroneManager::getInstance()); }
};