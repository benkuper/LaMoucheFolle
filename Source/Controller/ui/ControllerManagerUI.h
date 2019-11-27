/*
  ==============================================================================

    ControllerManagerUI.h
    Created: 30 Oct 2017 9:21:48am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "../ControllerManager.h"
#include "ControllerUI.h"

class ControllerManagerUI :
	public BaseManagerShapeShifterUI<ControllerManager, Controller, ControllerUI>
{
public:
	ControllerManagerUI(const String &name, ControllerManager * manager);
	~ControllerManagerUI();


	static ControllerManagerUI * create(const String &name) { return new ControllerManagerUI(name, ControllerManager::getInstance()); }


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllerManagerUI)
};