/*
  ==============================================================================

    Drone2DViewManagerUI.h
    Created: 25 Nov 2019 3:34:31pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once


#include "../DroneManager.h"
#include "Drone2DViewUI.h"

class DroneManager2DViewUI :
	public BaseManagerShapeShifterViewUI<DroneManager, Drone, Drone2DViewUI>
{
public:
	DroneManager2DViewUI(const String& name, DroneManager* manager);
	~DroneManager2DViewUI();

	void updateViewUIPosition(Drone2DViewUI * itemUI) override;

	static DroneManager2DViewUI* create(const String& name) { return new DroneManager2DViewUI(name, DroneManager::getInstance()); }
};