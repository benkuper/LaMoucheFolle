/*
  ==============================================================================

    DroneGridUI.h
    Created: 25 Nov 2019 3:34:12pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../Drone.h"
//#include "RadialMenuTarget.h"

class DroneGridUI :
	public BaseItemMinimalUI<Drone>
	//public RadialMenuTarget
{
public:
	DroneGridUI(Drone* drone);
	~DroneGridUI();

	Image droneImage;
	Image overlayImage;

	void paint(Graphics& g);
	void updateUI();

	void mouseDown(const MouseEvent& e) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;
	void containerChildAddressChangedAsync(ControllableContainer*) override;
};
