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
	addressLabel("DroneLabel"),
	stateFeedback(drone)
{
	inTriggerUI = item->inTrigger->createImageUI(CFAssetManager::getInstance()->getInImage());
	outTriggerUI = item->outTrigger->createImageUI(CFAssetManager::getInstance()->getOutImage());
	addAndMakeVisible(inTriggerUI);
	addAndMakeVisible(outTriggerUI);

	addressLabel.setText(drone->getRadioString(),dontSendNotification);
	addAndMakeVisible(&addressLabel);

	addAndMakeVisible(&stateFeedback);
}

DroneUI::~DroneUI()
{
}

void DroneUI::resizedInternalHeader(Rectangle<int>& r)
{
	stateFeedback.setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(10);

	outTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
	inTriggerUI->setBounds(r.removeFromRight(r.getHeight()));
	r.removeFromRight(10);

	addressLabel.setBounds(r.removeFromRight(jmin(200,r.getWidth()-60)));

}

void DroneUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	if (c == item->address || c == item->targetRadio || c == item->channel || c == item->speed)
	{
		addressLabel.setText(item->getRadioString() ,dontSendNotification);
	}
	else if (c == item->droneState)
	{
		DBG("Drone changed  state :" << (int)item->droneState->getValueData());
		stateFeedback.repaint();
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
	case Drone::READY: c = GREEN_COLOR; break;
	case Drone::ERROR: c = RED_COLOR; break;
	}
	
	g.setColour(c);
	g.fillEllipse(getLocalBounds().reduced(4).toFloat());
	g.setColour(c.brighter(.3f));
	g.drawEllipse(getLocalBounds().reduced(4).toFloat(),1);
}
