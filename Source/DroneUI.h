/*
  ==============================================================================

    DroneUI.h
    Created: 24 Oct 2017 11:19:31am
    Author:  Ben

  ==============================================================================
*/

#pragma once

/*
#include "Drone2.h"

class DroneStatusFeedback :
	public Component
{
public:
	DroneStatusFeedback(Drone2 *drone);
	~DroneStatusFeedback();

	Drone2 * drone;

	void paint(Graphics &g) override;

};


class DroneUI :
	public BaseItemUI<Drone2>
{
public:
	DroneUI(Drone2 * drone);
	~DroneUI();

	ScopedPointer<TriggerImageUI> inTriggerUI;
	ScopedPointer<TriggerImageUI> outTriggerUI;
	ScopedPointer<BoolImageToggleUI> chargingUI;
	ScopedPointer<BoolImageToggleUI> lowBatUI;
	ScopedPointer<FloatSliderUI> voltageUI;
	ScopedPointer<BoolImageToggleUI> flyingUI;
	ScopedPointer<TriggerButtonUI> connectUI;
	ScopedPointer<TriggerButtonUI> takeOffUI;
	ScopedPointer<ControllableUI> realPosUI;

	DroneStatusFeedback stateFeedback;

	void resizedInternalHeader(Rectangle<int> &r) override;
	void controllableFeedbackUpdateInternal(Controllable * c) override;
};

*/