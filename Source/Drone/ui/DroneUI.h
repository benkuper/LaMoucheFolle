/*
  ==============================================================================

    DroneUI.h
    Created: 25 Nov 2019 3:33:51pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../Drone.h"

class DroneUI :
	public BaseItemUI<Drone>
{
public:
	DroneUI(Drone* drone);
	~DroneUI();

	Rectangle<int> statusRect;

	void paint(Graphics& g) override;
	void resizedInternalHeader(Rectangle<int>& r) override;

	void controllableFeedbackUpdateInternal(Controllable* c) override;
	Colour getColorForState();
};


class VizImages
{
public:

	static Image getDroneStateImage(Drone* d);
	static Image getDroneOverlayImage(Drone* d);
};