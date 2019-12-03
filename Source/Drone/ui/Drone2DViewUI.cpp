/*
  ==============================================================================

    Drone2DViewUI.cpp
    Created: 25 Nov 2019 3:34:23pm
    Author:  bkupe

  ==============================================================================
*/

#include "Drone2DViewUI.h"
#include "DroneUI.h"

Drone2DViewUI::Drone2DViewUI(Drone* drone) :
	BaseItemMinimalUI(drone)/*,
	RadialMenuTarget(this)*/
{
	setSize(50, 50);
	dragAndDropEnabled = true;
	drawEmptyDragIcon = true; 
	autoHideWhenDragging = false;
	updateUI();
}

Drone2DViewUI::~Drone2DViewUI()
{
}

void Drone2DViewUI::paint(Graphics& g)
{
	Point<int> centre = getLocalBounds().getCentre();
	AffineTransform t = AffineTransform().translated(-centre).rotated(-(item->realRotation->z+90) * float_Pi / 180).translated(centre);
	g.addTransform(t);
	if (droneImage.getWidth() > 0) g.drawImage(droneImage, getLocalBounds().toFloat());
	if (overlayImage.getWidth() > 0) g.drawImage(overlayImage, getLocalBounds().reduced(20).toFloat());
	g.setColour(YELLOW_COLOR);
	g.addTransform(t.inverted());


	t = AffineTransform().translated(-centre).rotated(-item->getDesiredYaw()).translated(centre);
	g.addTransform(t);
	g.drawLine(centre.x, centre.y, centre.x + getWidth() / 2, centre.y, 2);
	g.addTransform(t.inverted());


	Rectangle<float> tr = getLocalBounds().reduced(20, 0).withHeight(14).toFloat();
	g.setColour(BG_COLOR.brighter().withAlpha(.6f));
	g.fillRoundedRectangle(tr, 4);
	g.setColour(BG_COLOR.brighter(.3f).withAlpha(.6f));
	g.drawRoundedRectangle(tr, 4, 1);
	g.setColour(TEXT_COLOR);
	g.drawFittedText(item->niceName, tr.toNearestInt(), Justification::centred, 1);
}

void Drone2DViewUI::updateUI()
{
	droneImage = VizImages::getDroneStateImage(item);
	overlayImage = VizImages::getDroneOverlayImage(item);
	repaint();
}

void Drone2DViewUI::mouseDown(const MouseEvent& e)
{
	BaseItemMinimalUI::mouseDown(e);
	if (e.mods.isLeftButtonDown())
	{
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
	else if (e.mods.isRightButtonDown())
	{
		yAtMouseDown = item->desiredPosition->y;
	}
	

}

void Drone2DViewUI::mouseDrag(const MouseEvent& e)
{
	if (e.mods.isRightButtonDown())
	{
		item->desiredPosition->setVector(item->desiredPosition->x, yAtMouseDown - e.getDistanceFromDragStartY() / 100.0f, item->desiredPosition->z);
	}

	BaseItemMinimalUI::mouseDrag(e);
}

void Drone2DViewUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	if (c == item->state || c == item->viewUIPosition || c == item->realRotation) updateUI();
}

void Drone2DViewUI::containerChildAddressChangedAsync(ControllableContainer*)
{
	repaint();
}