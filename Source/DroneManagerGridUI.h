/*
  ==============================================================================

    DroneManagerGridUI.h
    Created: 12 Jun 2018 3:59:57pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "DroneManager.h"
#include "DroneGridUI.h"

class DroneManagerGridUI :
	public BaseManagerShapeShifterUI<DroneManager, Drone, DroneGridUI>
{
public:
	DroneManagerGridUI(const String &name, DroneManager * manager);
	~DroneManagerGridUI();

	const int thumbSize = 128;
	const int headerHeight = 20;
	const int headerGap = 2;

	ScopedPointer<TriggerButtonUI> disableAllBT;
	ScopedPointer<TriggerButtonUI> enableAllBT;
	ScopedPointer<TriggerButtonUI> connectSelectedBT;
	ScopedPointer<TriggerButtonUI> disableNotFlyingsBT;
	ScopedPointer<TriggerButtonUI> connectAllBT;
	ScopedPointer<TriggerButtonUI> connectAllNCBT;
	ScopedPointer<TriggerButtonUI> resetKalmanBT;
	ScopedPointer<TriggerButtonUI> takeOffAllBT;
	ScopedPointer<TriggerButtonUI> landAllBT;
	ScopedPointer<TriggerButtonUI> unlockAllBT;

	void resized() override;

	//void resizedInternalHeader(Rectangle<int> &r) override;

	static DroneManagerGridUI * create(const String &name) { return new DroneManagerGridUI(name, DroneManager::getInstance()); }
};
