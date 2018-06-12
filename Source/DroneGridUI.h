/*
  ==============================================================================

    DroneGridUI.h
    Created: 12 Jun 2018 4:00:58pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Drone.h"
#include "RadialMenuTarget.h"

class DroneGridUI :
	public BaseItemMinimalUI<Drone>,
	public RadialMenuTarget
{
public:
	DroneGridUI(Drone * drone);
	~DroneGridUI();

	Image droneImage;
	Image overlayImage;

	void paint(Graphics &g);
	void updateUI();

	void controllableFeedbackUpdateInternal(Controllable * c) override;
};

class VizImages
{
public:

	static Image getDroneStateImage(Drone * d);
	static Image getDroneOverlayImage(Drone * d);
};