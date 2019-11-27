/*
  ==============================================================================

    DroneManagerUI.cpp
    Created: 25 Nov 2019 3:33:57pm
    Author:  bkupe

  ==============================================================================
*/

#include "DroneManagerUI.h"

DroneManagerUI::DroneManagerUI(const String& name, DroneManager* manager) :
	BaseManagerShapeShifterUI(name, manager)
{
	addExistingItems();
}

DroneManagerUI::~DroneManagerUI()
{

}