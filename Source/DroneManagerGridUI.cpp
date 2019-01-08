/*
  ==============================================================================

    DroneManagerGridUI.cpp
    Created: 12 Jun 2018 3:59:57pm
    Author:  Ben

  ==============================================================================
*/

#include "DroneManagerGridUI.h"

DroneManagerGridUI::DroneManagerGridUI(const String & name, CFDroneManager * manager) :
	BaseManagerShapeShifterUI(name, manager, true)
{
	animateItemOnAdd = false;
	setSize(100, 100);

	addExistingItems();
}

DroneManagerGridUI::~DroneManagerGridUI()
{
}

void DroneManagerGridUI::resized()
{

	Rectangle<int> r = getLocalBounds().reduced(2);
	if (r.getWidth() == 0) return;
	addItemBT->setBounds(r.withSize(headerHeight, headerHeight).withX(r.getWidth()-headerHeight));

	if (itemsUI.size() > 0)
	{
		//container.setBounds(getLocalBounds());
		//viewport.setBounds(getLocalBounds());
	}

	int numThumbs = itemsUI.size();
	int numThumbPerLine = jmin(r.getWidth() / (thumbSize + gap), numThumbs);
	int numLines = ceil(numThumbs*1.f / numThumbPerLine);

	r.setHeight(numLines * (thumbSize + gap) - gap);

	int index = 0;
	int yIndex = 0;

	Rectangle<int> lr;

	for (auto &mui : itemsUI)
	{
		if (index % numThumbPerLine == 0)
		{

			int numThumbsInThisLine = jmin(numThumbs - index, numThumbPerLine);
			int lineWidth = numThumbsInThisLine * (thumbSize + gap) - gap;

			if (yIndex > 0) r.removeFromTop(gap);
			lr = r.removeFromTop(thumbSize);
			lr = lr.withSizeKeepingCentre(lineWidth, lr.getHeight());

			yIndex++;
		}

		mui->setBounds(lr.removeFromLeft(thumbSize));
		lr.removeFromLeft(gap);
		index++;
	}

	viewport.setBounds(getLocalBounds().withTrimmedTop(headerHeight+headerGap));
	container.setSize(getWidth(), r.getBottom());
	//setSize(getWidth(), r.getBottom());
}
