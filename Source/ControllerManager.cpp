/*
  ==============================================================================

    OutputManager.cpp
    Created: 19 Oct 2017 7:35:51pm
    Author:  Ben

  ==============================================================================
*/


#include "OSCController.h"
#include "NodeManager.h"
#include "ControllerManager.h"

juce_ImplementSingleton(ControllerManager)
juce_ImplementSingleton(ControllerFactory);

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

void ControllerManager::sendFullSetup()
{
	for (Controller * i : items) i->sendFullSetup();
}

void ControllerManager::sendDroneFeedback(Drone * d, Controllable * c)
{
	for (Controller * i : items) i->sendDroneFeedback(d, c);
}

void ControllerManager::sendNodeFeedback(Node * n, Controllable * c)
{
	for (Controller * i : items) i->sendNodeFeedback(n, c);
}


void ControllerManager::controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
 	Drone * d = dynamic_cast<Drone *>(c->parentContainer);
	if (d != nullptr)
	{
		//if(c != d->realPosition && c != d->inTrigger && c != d->outTrigger) LOG("Feedback : " << c->shortName);
		if (c == d->realPosition || c == d->voltage || c == d->lowBattery || c == d->charging || c == d->droneState || c == d->headlight || c == d->color || c == d->lightMode)
		{
			sendDroneFeedback(d, c);
		}
		return;
	}

	Node * n = dynamic_cast<Node *>(c->parentContainer);
	if (n != nullptr)
	{
		if (c == n->id || c == n->position)
		{
			sendNodeFeedback(n, c);
		}
		return;
	}
}

ControllerFactory::ControllerFactory()
{
	defs.add(Definition::createDef("", "OSC", &OSCController::create));
}
