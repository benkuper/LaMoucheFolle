/*
  ==============================================================================

    DroneManagerGridUI.h
    Created: 12 Jun 2018 3:59:57pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "CFDroneManager.h"
#include "DroneGridUI.h"

class DroneManagerGridUI :
	public BaseManagerShapeShifterUI<CFDroneManager, CFDrone, DroneGridUI>
{
public:
	DroneManagerGridUI(const String &name, CFDroneManager * manager);
	~DroneManagerGridUI();

	const int thumbSize = 128;
	const int headerHeight = 20;
	const int headerGap = 2;

	std::unique_ptr<TriggerButtonUI> disableAllBT;
	std::unique_ptr<TriggerButtonUI> enableAllBT;
	std::unique_ptr<TriggerButtonUI> connectSelectedBT;
	std::unique_ptr<TriggerButtonUI> disableNotFlyingsBT;
	std::unique_ptr<TriggerButtonUI> connectAllBT;
	std::unique_ptr<TriggerButtonUI> connectAllNCBT;
	std::unique_ptr<TriggerButtonUI> resetKalmanBT;
	std::unique_ptr<TriggerButtonUI> takeOffAllBT;
	std::unique_ptr<TriggerButtonUI> landAllBT;
	std::unique_ptr<TriggerButtonUI> unlockAllBT;

	void resized() override;

	//void resizedInternalHeader(Rectangle<int> &r) override;

	static DroneManagerGridUI * create(const String &name) { return new DroneManagerGridUI(name, CFDroneManager::getInstance()); }
};
