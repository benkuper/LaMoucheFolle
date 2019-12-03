/*
  ==============================================================================

    Deck.cpp
    Created: 29 Nov 2019 10:56:22am
    Author:  bkupe

  ==============================================================================
*/

#include "Deck.h"

Deck::Deck() :
	BaseItem("Deck"),
	paramManager("Parameters", false, false, false)
{
	saveAndLoadRecursiveData = true;
	addChildControllableContainer(&paramManager);
}

Deck::~Deck()
{
}
