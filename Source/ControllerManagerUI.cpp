/*
  ==============================================================================

    ControllerManagerUI.cpp
    Created: 30 Oct 2017 9:21:48am
    Author:  Ben

  ==============================================================================
*/

#include "ControllerManagerUI.h"

ControllerManagerUI::ControllerManagerUI(const String &name, ControllerManager * manager) :
	BaseManagerShapeShifterUI(name,manager)
{
	addExistingItems();
}

ControllerManagerUI::~ControllerManagerUI()
{
}
