/*
  ==============================================================================

    DroneManagerGridUI.cpp
    Created: 25 Nov 2019 3:34:02pm
    Author:  bkupe

  ==============================================================================
*/

#include "DroneManagerGridUI.h"

DroneManagerGridUI::DroneManagerGridUI(const String& name, DroneManager* manager) :
	BaseManagerShapeShifterUI(name, manager, true)
{
	manager->addAsyncContainerListener(this);

	animateItemOnAdd = false;

	thumbSizeUI.reset(manager->thumbSize->createSlider());
	takeOffAllBT.reset(manager->takeOffAll->createButtonUI());
	landAllBT.reset(manager->landAll->createButtonUI());

	addAndMakeVisible(thumbSizeUI.get());
	addAndMakeVisible(takeOffAllBT.get());
	addAndMakeVisible(landAllBT.get());

	setSize(100, 100);
	addExistingItems();
}

DroneManagerGridUI::~DroneManagerGridUI()
{
	if(!inspectable.wasObjectDeleted()) manager->removeAsyncContainerListener(this);
}

void DroneManagerGridUI::resizedInternalHeader(Rectangle<int>& r)
{
	Rectangle<int> hr = r.removeFromTop(headerHeight);
	BaseManagerShapeShifterUI::resizedInternalHeader(hr);

	thumbSizeUI->setBounds(hr.removeFromRight(100).reduced(1));
	r.removeFromRight(2);

	takeOffAllBT->setBounds(hr.removeFromLeft(100).reduced(1));
	r.removeFromLeft(8);
	landAllBT->setBounds(hr.removeFromLeft(100).reduced(1));

	r.removeFromTop(headerGap);

}

void DroneManagerGridUI::placeItems(Rectangle<int> &r)
{
	int thumbSize = manager->thumbSize->intValue();
	int numThumbs = itemsUI.size();
	int numThumbPerLine = jmax(jmin(numThumbs, r.getWidth() / (thumbSize + gap)), 1);
	int numLines = ceil(numThumbs * 1.f / numThumbPerLine);

	r.setHeight(numLines * (thumbSize + gap) - gap);

	int index = 0;
	int yIndex = 0;

	Rectangle<int> lr;

	for (auto& mui : itemsUI)
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
}

void DroneManagerGridUI::newMessage(const ContainerAsyncEvent& e)
{
	switch (e.type)
	{
	case ContainerAsyncEvent::ControllableFeedbackUpdate:
		if (e.targetControllable == manager->thumbSize)
		{
			resized();
		}
		break;
	}
}
