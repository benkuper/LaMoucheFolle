/*
  ==============================================================================

    DroneUI.cpp
    Created: 24 Oct 2017 11:19:31am
    Author:  Ben

  ==============================================================================
*/

#include "DroneUI.h"
#include "CFAssetManager.h"

DroneUI::DroneUI(Drone * drone) :
	BaseItemUI(drone)
{
	inTriggerUI = item->inTrigger->createImageUI(CFAssetManager::getInstance()->getInImage());
	outTriggerUI = item->outTrigger->createImageUI(CFAssetManager::getInstance()->getOutImage());
	addAndMakeVisible(inTriggerUI);
	addAndMakeVisible(outTriggerUI);
}

DroneUI::~DroneUI()
{
}

void DroneUI::resizedInternalHeader(Rectangle<int>& r)
{
	outTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
	inTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
}

