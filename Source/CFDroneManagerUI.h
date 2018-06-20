/*
  ==============================================================================

    CFDroneManagerUI.h
    Created: 19 Jun 2018 8:40:01am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "CFDroneManager.h"

class CFDroneManagerUI :
	public BaseManagerShapeShifterUI<CFDroneManager, CFDrone, BaseItemUI<CFDrone>>
{
public:
	CFDroneManagerUI(const String &name, CFDroneManager * manager) : BaseManagerShapeShifterUI(name, manager) {}
	~CFDroneManagerUI() {}

	static CFDroneManagerUI * create(const String &name) { return new CFDroneManagerUI(name, CFDroneManager::getInstance()); }
};
