/*
  ==============================================================================

    Controller.h
    Created: 30 Oct 2017 9:21:36am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

#include "Drone.h"

class Controller :
	public BaseItem
{
public:
	Controller(const String &name = "Controller");
	~Controller();

	BoolParameter * logIncomingData;
	BoolParameter * logOutgoingData;

	Trigger * inTrigger;
	Trigger * outTrigger;

	virtual void sendFeedback(Drone * d, Controllable * c) {}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
};