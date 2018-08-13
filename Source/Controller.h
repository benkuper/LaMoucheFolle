/*
  ==============================================================================

    Controller.h
    Created: 30 Oct 2017 9:21:36am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

#include "CFDrone.h"
#include "Node.h"

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

	virtual void sendDroneFeedback(CFDrone * d, Controllable * c) {}
	virtual void sendNodeFeedback(Node * d, Controllable * c) {}

	virtual void sendFullSetup() {} 
	virtual void sendDroneSetup(const String &droneName) {}
	virtual void sendNodeSetup(const String &nodeName) {}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
};