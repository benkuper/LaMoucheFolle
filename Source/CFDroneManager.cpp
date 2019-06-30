/*
  ==============================================================================

	CFDroneManager.cpp
	Created: 19 Jun 2018 8:39:09am
	Author:  Ben

  ==============================================================================
*/

#include "CFDroneManager.h"
//#include "CFLog.h"
juce_ImplementSingleton(CFDroneManager)

CFDroneManager::CFDroneManager() :
	BaseManager("CF Drones") 
{
	//CFParamToc::loadParamTocs();
	//CFLogToc::loadLogTocs();
}

CFDroneManager::~CFDroneManager() {}
