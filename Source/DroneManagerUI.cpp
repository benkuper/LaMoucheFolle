/*
  ==============================================================================

    DroneManagerUI.cpp
    Created: 24 Oct 2017 11:19:35am
    Author:  Ben

  ==============================================================================
*/

#include "DroneManagerUI.h"

DroneManagerUI::DroneManagerUI(const String & name, DroneManager * manager) :
	BaseManagerShapeShifterUI(name, manager)
{
	disableAllBT = manager->disableAllTrigger->createButtonUI();
	enableAllBT = manager->enableAllTrigger->createButtonUI();
	connectAllBT = manager->connectAllTrigger->createButtonUI();
	connectSelectedBT = manager->connectSelectedTrigger->createButtonUI();
	disableNotFlyingsBT = manager->disableNotFlyingsTrigger->createButtonUI();
	connectAllNCBT = manager->connectAllNotConnectedTrigger->createButtonUI();
	resetKalmanBT = manager->resetAllKalman->createButtonUI();

	addAndMakeVisible(connectAllBT);
	addAndMakeVisible(connectAllNCBT);
	addAndMakeVisible(resetKalmanBT);
	addAndMakeVisible(disableAllBT);
	addAndMakeVisible(enableAllBT);
	addAndMakeVisible(connectSelectedBT);
	addAndMakeVisible(disableNotFlyingsBT);
	addExistingItems();
}

DroneManagerUI::~DroneManagerUI()
{
}

void DroneManagerUI::resizedInternalHeader(Rectangle<int>& r)
{
	Rectangle<int> h = r.removeFromTop(40);
	Rectangle<int> h1 = h.removeFromTop(20).reduced(2);
	Rectangle<int> h2 = h.reduced(2);

	disableAllBT->setBounds(h1.removeFromLeft(60));
	h1.removeFromLeft(10);
	enableAllBT->setBounds(h1.removeFromLeft(60));
	h1.removeFromLeft(10);
	connectAllBT->setBounds(h1.removeFromLeft(70));
	h1.removeFromLeft(10);
	connectAllNCBT->setBounds(h1.removeFromLeft(130));
	
	connectSelectedBT->setBounds(h2.removeFromLeft(100));
	h2.removeFromLeft(10);
	disableNotFlyingsBT->setBounds(h2.removeFromLeft(100));
	h2.removeFromLeft(10);
	resetKalmanBT->setBounds(h2.removeFromLeft(100));
}
