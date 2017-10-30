/*
  ==============================================================================

    OutputManager.cpp
    Created: 19 Oct 2017 7:35:51pm
    Author:  Ben

  ==============================================================================
*/

#include "ControllerManager.h"

juce_ImplementSingleton(ControllerManager)
juce_ImplementSingleton(ControllerFactory);

#include "OSCController.h"

ControllerManager::ControllerManager() :
	BaseManager("Controllers")
{
	managerFactory = ControllerFactory::getInstance();
}

ControllerManager::~ControllerManager()
{
	ControllerFactory::deleteInstance();
}

ControllerFactory::ControllerFactory()
{
	defs.add(Definition::createDef("", "OSC", &OSCController::create));
}
