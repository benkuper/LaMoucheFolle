/*
  ==============================================================================

    DroneGridUI.h
    Created: 12 Jun 2018 4:00:58pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "CFDrone.h"
#include "RadialMenuTarget.h"

class DroneGridUI :
	public BaseItemMinimalUI<CFDrone>,
	public RadialMenuTarget
{
public:
	DroneGridUI(CFDrone * drone);
	~DroneGridUI();

	Image droneImage;
	Image overlayImage;

	void paint(Graphics &g);
	void updateUI();

	void mouseDown(const MouseEvent &e) override;

	void controllableFeedbackUpdateInternal(Controllable * c) override;
	void containerChildAddressChangedAsync(ControllableContainer *) override;
};

class VizImages
{
public:

	static Image getDroneStateImage(CFDrone * d);
	static Image getDroneOverlayImage(CFDrone * d);
};