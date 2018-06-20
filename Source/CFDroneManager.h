/*
  ==============================================================================

	CFDroneManager.h
	Created: 19 Jun 2018 8:39:09am
	Author:  Ben

  ==============================================================================
*/

#pragma once

#include "CFDrone.h"

class CFDroneManager :
	public BaseManager<CFDrone>
{
public:
	juce_DeclareSingleton(CFDroneManager, true)
	CFDroneManager();
	~CFDroneManager();
};