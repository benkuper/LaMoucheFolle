/*
  ==============================================================================

    DroneGridUI.cpp
    Created: 25 Nov 2019 3:34:12pm
    Author:  bkupe

  ==============================================================================
*/

#include "DroneGridUI.h"
#include "DroneUI.h"

DroneGridUI::DroneGridUI(Drone* drone) :
	BaseItemMinimalUI(drone)/*,
	RadialMenuTarget(this)*/
{
	updateUI();
}

DroneGridUI::~DroneGridUI()
{
}

void DroneGridUI::paint(Graphics& g)
{
	if (droneImage.getWidth() > 0) g.drawImage(droneImage, getLocalBounds().toFloat());
	if (overlayImage.getWidth() > 0) g.drawImage(overlayImage, getLocalBounds().reduced(20).toFloat());

	//Progress
	/*
	Drone::State s = item->state->getValueDataAsEnum<Drone::State>();
	float progress = 0;
	switch (s)
	{
	case Drone::CALIBRATING: progress = item->calibrationProgress->floatValue(); break;
	case Drone::HEALTH_CHECK: progress = item->analysisProgress->floatValue(); break;
	}

	if (progress > 0)
	{
		Path p;
		p.addArc(10, 10, getWidth() - 20, getHeight() - 20, 0, float_Pi * 2 * progress, true);

		g.setColour(Colours::yellow);
		g.strokePath(p, PathStrokeType(4));
	}
	*/

	Rectangle<float> tr = getLocalBounds().reduced(20, 0).withHeight(14).toFloat();
	g.setColour(BG_COLOR.brighter().withAlpha(.6f));
	g.fillRoundedRectangle(tr, 4);
	g.setColour(BG_COLOR.brighter(.3f).withAlpha(.6f));
	g.drawRoundedRectangle(tr, 4, 1);
	g.setColour(TEXT_COLOR);
	g.drawFittedText(item->niceName, tr.toNearestInt(), Justification::centred, 1);
}

void DroneGridUI::updateUI()
{
	droneImage = VizImages::getDroneStateImage(item);
	overlayImage = VizImages::getDroneOverlayImage(item);
	repaint();
}

void DroneGridUI::mouseDown(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDown(e);
	if (e.mods.isShiftDown())
	{
		item->takeOffTrigger->trigger();
	}
	else if (e.mods.isCommandDown())
	{
		item->landTrigger->trigger();
	}
	else if (e.mods.isAltDown())
	{
		item->stopTrigger->trigger();
	}

}

void DroneGridUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == item->state) updateUI();
}

void DroneGridUI::containerChildAddressChangedAsync(ControllableContainer*)
{
	repaint();
}


