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
	DroneManager::getInstance()->addControllableContainerListener(this);
}

ControllerManager::~ControllerManager()
{
	if (DroneManager::getInstanceWithoutCreating() != nullptr) DroneManager::getInstance()->removeControllableContainerListener(this);
	ControllerFactory::deleteInstance();
}

void ControllerManager::sendDroneFeedback(Drone * d, Controllable * c)
{
	for (Controller * i : items) i->sendFeedback(d, c);
}

void ControllerManager::onExternalParameterChanged(Parameter * p)
{
	DBG("External parameter changed");
}

void ControllerManager::controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
	Drone * d = reinterpret_cast<Drone *>(cc);
	if (d != nullptr)
	{
		if (c == d->realPosition || c == d->lowBattery || c == d->charging || c == d->droneState)
		{
			sendDroneFeedback(d, c);
		}
	}
}

ControllerFactory::ControllerFactory()
{
	defs.add(Definition::createDef("", "OSC", &OSCController::create));
}
