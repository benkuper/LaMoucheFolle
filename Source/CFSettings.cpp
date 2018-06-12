/*
  ==============================================================================

    CFSettings.cpp
    Created: 12 Jun 2018 10:19:04pm
    Author:  Ben

  ==============================================================================
*/

#include "CFSettings.h"
juce_ImplementSingleton(CFSettings)

CFSettings::CFSettings() :
	ControllableContainer("Crazyflie settings"),
	flightCC("Flight"),
	miscCC("Miscellaneous")
{
	addChildControllableContainer(&flightCC);
	flightSpeedFactor = flightCC.addFloatParameter("Flight Speed Factor", "Aggressiveness of the flight", 1, 1, 10);

	addChildControllableContainer(&miscCC);
	zIsVertical = miscCC.addBoolParameter("Z is vertical", "If checked, will use z as vertical axis", false);
}


CFSettings::~CFSettings()
{
}
