/*
  ==============================================================================

    CFAssetManager.h
    Created: 25 Oct 2017 11:24:57am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class CFAssetManager
{
public:
	juce_DeclareSingleton(CFAssetManager, true);
	CFAssetManager();
	virtual ~CFAssetManager();

	ImageButton * getSetupBTImage(const Image & image);
	ImageButton * getToggleBTImage(const Image &image);
};


