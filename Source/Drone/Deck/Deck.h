/*
  ==============================================================================

    Deck.h
    Created: 29 Nov 2019 10:56:22am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class Deck :
	public BaseItem
{
public:
	Deck();
	~Deck();

	GenericControllableManager paramManager;
};