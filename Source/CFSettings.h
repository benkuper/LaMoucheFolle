/*
  ==============================================================================

    CFSettings.h
    Created: 12 Jun 2018 10:19:04pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class CFSettings : public ControllableContainer
{
public:
	CFSettings();
	~CFSettings();

	juce_DeclareSingleton(CFSettings, true)

	ControllableContainer flightCC;
	FloatParameter * flightSpeedFactor;

	ControllableContainer miscCC;
	BoolParameter * zIsVertical;
};
