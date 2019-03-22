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
#include "CFDroneManager.h"

juce_ImplementSingleton(ControllerManager)
juce_ImplementSingleton(ControllerFactory);

ControllerManager::ControllerManager() :
	BaseManager("Controllers")
{
	managerFactory = ControllerFactory::getInstance();
	CFDroneManager::getInstance()->addControllableContainerListener(this);
}

ControllerManager::~ControllerManager()
{
	if (CFDroneManager::getInstanceWithoutCreating() != nullptr) CFDroneManager::getInstance()->removeControllableContainerListener(this);
	ControllerFactory::deleteInstance();
}

void ControllerManager::sendFullSetup()
{
	for (Controller * i : items) i->sendFullSetup();
}

void ControllerManager::sendDroneFeedback(CFDrone * d, Controllable * c)
{
	for (Controller * i : items) i->sendDroneFeedback(d, c);
}

void ControllerManager::sendNodeFeedback(Node * n, Controllable * c)
{
	for (Controller * i : items) i->sendNodeFeedback(n, c);
}


void ControllerManager::controllableFeedbackUpdate(ControllableContainer * cc, Controllable * c)
{
	if (dynamic_cast<Controller *>(c->parentContainer.get()))
	{
		return;
	}

	CFDrone * d = ControllableUtil::findParentAs<CFDrone>(c);
	if (d != nullptr)
	{
		//if(c != d->realPosition && c != d->inTrigger && c != d->outTrigger) LOG("Feedback : " << c->shortName);
		sendDroneFeedback(d, c);
		return;
	} else
	{
		//DBG("Not found for " << c->getControlAddress());
	}

	Node * n = dynamic_cast<Node *>(c->parentContainer.get());
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
