/*
  ==============================================================================

    DroneUI.h
    Created: 24 Oct 2017 11:19:31am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Drone.h"

class DroneUI :
	public BaseItemUI<Drone>
{
public:
	DroneUI(Drone * drone);
	~DroneUI();

	ScopedPointer<TriggerImageUI> inTriggerUI;
	ScopedPointer<TriggerImageUI> outTriggerUI;

	void resizedInternalHeader(Rectangle<int> &r) override;
};