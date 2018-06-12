/*
  ==============================================================================

    DroneUI.cpp
    Created: 24 Oct 2017 11:19:31am
    Author:  Ben

  ==============================================================================
*/

/*
#include "DroneUI.h"
#include "CFAssetManager.h"

DroneUI::DroneUI(Drone2 * drone) :
	BaseItemUI(drone),
	stateFeedback(drone)
{
	//inTriggerUI = item->inTrigger->createImageUI(CFAssetManager::getInstance()->getInImage());
	//outTriggerUI = item->outTrigger->createImageUI(CFAssetManager::getInstance()->getOutImage());
	lowBatUI = item->lowBattery->createImageToggle(CFAssetManager::getInstance()->getToggleBTImage(CFAssetManager::getInstance()->getLowBatteryImage()));
	//chargingUI = item->charging->createImageToggle(CFAssetManager::getInstance()->getToggleBTImage(CFAssetManager::getInstance()->getChargingImage()));
	voltageUI = item->voltage->createSlider();
	//flyingUI = item->isFlying->createImageToggle(CFAssetManager::getInstance()->getToggleBTImage(CFAssetManager::getInstance()->getFlyingImage()));
	connectUI = item->connectTrigger->createButtonUI();
	takeOffUI = item->takeOffTrigger->createButtonUI();
	realPosUI = item->realPosition->createDefaultUI();

	addAndMakeVisible(inTriggerUI);
	addAndMakeVisible(outTriggerUI);
	addAndMakeVisible(lowBatUI);
	addAndMakeVisible(chargingUI);
	addAndMakeVisible(voltageUI);
	addAndMakeVisible(flyingUI);
	addAndMakeVisible(connectUI);
	addAndMakeVisible(takeOffUI);
	addAndMakeVisible(realPosUI);

	addAndMakeVisible(&stateFeedback);

	//voltageUI->setFrontColor(item->charging->boolValue() ? YELLOW_COLOR : (item->voltage->getNormalizedValue() < .1f ? RED_COLOR : BLUE_COLOR));
}

DroneUI::~DroneUI()
{
}


void DroneUI::resizedInternalHeader(Rectangle<int>& r)
{
	stateFeedback.setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(2);
	//flyingUI->setBounds(r.removeFromRight(r.getHeight()));
	//r.removeFromRight(2);
	lowBatUI->setBounds(r.removeFromRight(r.getHeight()));
	//chargingUI->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(10);


	//outTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
	//inTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
	//r.removeFromRight(10);

	connectUI->setBounds(r.removeFromRight(50).reduced(0, 1));
	r.removeFromRight(2);
	takeOffUI->setBounds(r.removeFromRight(50).reduced(0, 1));
	r.removeFromRight(2);
	realPosUI->setBounds(r.removeFromRight(100).reduced(0, 1));
	r.removeFromRight(6);
	voltageUI->setBounds(r.removeFromRight(r.getWidth() - 100).reduced(0,1));
	
}

void DroneUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	if (c == item->lowBattery || c == item->voltage)
	{
		voltageUI->setFrontColor((item->voltage->getNormalizedValue() < .1f ? RED_COLOR : BLUE_COLOR);
	}
	else if (c == item->state)
	{
		//DBG("Drone changed  state :" << (int)item->droneState->getValueData());
		stateFeedback.repaint();
	}
}


DroneStatusFeedback::DroneStatusFeedback(Drone2 * drone) :
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
	Drone2::DroneState s = drone->state->getValueDataAsEnum<Drone2::DroneState>();

	switch(s)
	{
	case Drone2::DISCONNECTED: c = BG_COLOR.brighter(.3f); break;
	case Drone2::CONNECTING: c = BLUE_COLOR; break;
	case Drone2::READY: c = GREEN_COLOR; break;
	case Drone2::CALIBRATING: c = Colours::purple.brighter(.3f); break;
	case Drone2::TAKEOFF: c = Colours::orange;
	case Drone2::FLYING: c = Colours::yellow;
	case Drone2::ERROR: c = RED_COLOR; break;
	}
	
	g.setColour(c);
	g.fillEllipse(getLocalBounds().reduced(4).toFloat());
	g.setColour(c.brighter(.3f));
	g.drawEllipse(getLocalBounds().reduced(4).toFloat(),1);
}

*/