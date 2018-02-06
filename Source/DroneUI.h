/*
  ==============================================================================

    DroneUI.h
    Created: 24 Oct 2017 11:19:31am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Drone.h"

class DroneStatusFeedback :
	public Component
{
public:
	DroneStatusFeedback(Drone *drone);
	~DroneStatusFeedback();

	Drone * drone;

	void paint(Graphics &g) override;

};


class DroneUI :
	public BaseItemUI<Drone>
{
public:
	DroneUI(Drone * drone);
	~DroneUI();

	ScopedPointer<TriggerImageUI> inTriggerUI;
	ScopedPointer<TriggerImageUI> outTriggerUI;
	ScopedPointer<BoolImageToggleUI> chargingUI;
	ScopedPointer<BoolImageToggleUI> lowBatUI;
	ScopedPointer<FloatSliderUI> voltageUI;
	ScopedPointer<BoolImageToggleUI> flyingUI;
	ScopedPointer<TriggerButtonUI> connectUI;

	DroneStatusFeedback stateFeedback;

	void resizedInternalHeader(Rectangle<int> &r) override;

	void controllableFeedbackUpdateInternal(Controllable * c) override;
};
