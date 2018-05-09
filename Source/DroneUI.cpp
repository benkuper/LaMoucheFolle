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
	BaseItemUI(drone),
	stateFeedback(drone)
{
	inTriggerUI = item->inTrigger->createImageUI(CFAssetManager::getInstance()->getInImage());
	outTriggerUI = item->outTrigger->createImageUI(CFAssetManager::getInstance()->getOutImage());
	lowBatUI = item->lowBattery->createImageToggle(CFAssetManager::getInstance()->getToggleBTImage(CFAssetManager::getInstance()->getLowBatteryImage()));
	chargingUI = item->charging->createImageToggle(CFAssetManager::getInstance()->getToggleBTImage(CFAssetManager::getInstance()->getChargingImage()));
	voltageUI = item->voltage->createSlider();
	flyingUI = item->isFlying->createImageToggle(CFAssetManager::getInstance()->getToggleBTImage(CFAssetManager::getInstance()->getFlyingImage()));
	connectUI = item->connectTrigger->createButtonUI();

	addAndMakeVisible(inTriggerUI);
	addAndMakeVisible(outTriggerUI);
	addAndMakeVisible(lowBatUI);
	addAndMakeVisible(chargingUI);
	addAndMakeVisible(voltageUI);
	addAndMakeVisible(flyingUI);
	addAndMakeVisible(connectUI);

	addAndMakeVisible(&stateFeedback);

	voltageUI->setFrontColor(item->charging->boolValue() ? YELLOW_COLOR : (item->voltage->getNormalizedValue() < .1f ? RED_COLOR : BLUE_COLOR));
}

DroneUI::~DroneUI()
{
}

void DroneUI::resizedInternalHeader(Rectangle<int>& r)
{
	stateFeedback.setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);
	flyingUI->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);
	lowBatUI->setBounds(r.removeFromRight(r.getHeight()));
	chargingUI->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(10);

	outTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
	inTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(10);

	connectUI->setBounds(r.removeFromRight(50).reduced(0, 1));
	r.removeFromRight(6);
	voltageUI->setBounds(r.removeFromRight(r.getWidth() - 100).reduced(0,1));
	
}

void DroneUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	if (c == item->charging || c == item->lowBattery || c == item->voltage)
	{
		voltageUI->setFrontColor(item->charging->boolValue() ? YELLOW_COLOR : (item->voltage->getNormalizedValue() < .1f ? RED_COLOR : BLUE_COLOR));
	}
	else if (c == item->droneState)
	{
		DBG("Drone changed  state :" << (int)item->droneState->getValueData());
		stateFeedback.repaint();
	} else if (c == item->launchingMode)
	{
		bgColor = item->launchingMode->boolValue() ? Colours::rebeccapurple : BG_COLOR.brighter(.1f);
		repaint();
	}
}


DroneStatusFeedback::DroneStatusFeedback(Drone * drone) :
	drone(drone)	
{
}

DroneStatusFeedback::~DroneStatusFeedback()
{
}

void DroneStatusFeedback::paint(Graphics & g)
{
	if (drone == nullptr) return;

	Colour c = Colours::black;
	Drone::DroneState s = drone->droneState->getValueDataAsEnum<Drone::DroneState>();

	switch(s)
	{
	case Drone::DISCONNECTED: c = BG_COLOR.brighter(.3f); break;
	case Drone::CONNECTING: c = BLUE_COLOR; break;
	case Drone::STABILIZING: c = Colours::purple.brighter(.3f); break;
	case Drone::READY: c = GREEN_COLOR; break;
	case Drone::ERROR: c = RED_COLOR; break;
	}
	
	g.setColour(c);
	g.fillEllipse(getLocalBounds().reduced(4).toFloat());
	g.setColour(c.brighter(.3f));
	g.drawEllipse(getLocalBounds().reduced(4).toFloat(),1);
}
