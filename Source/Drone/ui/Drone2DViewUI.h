/*
  ==============================================================================

    Drone2DViewUI.h
    Created: 25 Nov 2019 3:34:23pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../Drone.h"

class Drone2DViewUI :
	public BaseItemMinimalUI<Drone>
	//public RadialMenuTarget
{
public:
	Drone2DViewUI(Drone* drone);
	~Drone2DViewUI();

	Image droneImage;
	Image overlayImage;

	float yAtMouseDown;

	void paint(Graphics& g);
	void updateUI();

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;


	void controllableFeedbackUpdateInternal(Controllable* c) override;
	void containerChildAddressChangedAsync(ControllableContainer*) override;
};
