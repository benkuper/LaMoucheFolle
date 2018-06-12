/*
  ==============================================================================

    DroneManagerUI.h
    Created: 24 Oct 2017 11:19:35am
    Author:  Ben

  ==============================================================================
*/

/*
#pragma once
#include "DroneManager.h"
#include "DroneUI.h"

class DroneManagerUI :
	public BaseManagerShapeShifterUI<DroneManager, Drone2, DroneUI>
{
public:
	DroneManagerUI(const String &name, DroneManager * manager);
	~DroneManagerUI();

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

	void resizedInternalHeader(Rectangle<int> &r) override;

	static DroneManagerUI * create(const String &name) { return new DroneManagerUI(name, DroneManager::getInstance()); }
};
*/