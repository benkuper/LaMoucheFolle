/*
  ==============================================================================

    Drone2DViewManagerUI.cpp
    Created: 25 Nov 2019 3:34:31pm
    Author:  bkupe

  ==============================================================================
*/

#include "Drone2DViewManagerUI.h"

DroneManager2DViewUI::DroneManager2DViewUI(const String& name, DroneManager* manager) :
	BaseManagerShapeShifterViewUI(name, manager)
{
	
	updatePositionOnDragMove = true;
	addExistingItems();
}

DroneManager2DViewUI::~DroneManager2DViewUI()
{
}

void DroneManager2DViewUI::updateViewUIPosition(Drone2DViewUI* itemUI)
{
	BaseManagerShapeShifterViewUI::updateViewUIPosition(itemUI);
}
