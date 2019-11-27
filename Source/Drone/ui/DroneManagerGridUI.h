/*
  ==============================================================================

    DroneManagerGridUI.h
    Created: 25 Nov 2019 3:34:02pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "../DroneManager.h"
#include "DroneGridUI.h"

class DroneManagerGridUI :
	public BaseManagerShapeShifterUI<DroneManager, Drone, DroneGridUI>,
	public ContainerAsyncListener
{
public:
	DroneManagerGridUI(const String& name,DroneManager* manager);
	~DroneManagerGridUI();
	
	const int headerHeight = 20;
	const int headerGap = 2;

	std::unique_ptr<IntSliderUI> thumbSizeUI;
	std::unique_ptr<TriggerButtonUI> takeOffAllBT;
	std::unique_ptr<TriggerButtonUI> landAllBT;

	void resizedInternalHeader(Rectangle<int>& r) override;
	void placeItems(Rectangle<int> &r) override;

	void newMessage(const ContainerAsyncEvent& e) override;

	static DroneManagerGridUI* create(const String& name) { return new DroneManagerGridUI(name, DroneManager::getInstance()); }
};
