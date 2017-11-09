/*
  ==============================================================================

    Controller.cpp
    Created: 30 Oct 2017 9:21:36am
    Author:  Ben

  ==============================================================================
*/

#include "Controller.h"

Controller::Controller(const String &name) :
	BaseItem(name)
{
	logIncomingData = addBoolParameter("Log Incoming", "Enable / Disable logging of incoming data for this module", false);
	logIncomingData->hideInOutliner = true;
	logIncomingData->isTargettable = false;

	logOutgoingData = addBoolParameter("Log Outgoing", "Enable / Disable logging of outgoing data for this module", false);
	logOutgoingData->hideInOutliner = true;
	logOutgoingData->isTargettable = false;

	inTrigger = addTrigger("IN Activity", "Incoming Activity Signal");
	inTrigger->hideInEditor = true;

	outTrigger = addTrigger("OUT Activity", "Outgoing Activity Signal");
	outTrigger->hideInEditor = true;

}

Controller::~Controller()
{

}
