/*
  ==============================================================================

    DeckManager.h
    Created: 29 Nov 2019 10:56:29am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include  "Deck.h"

class DeckManager :
	public BaseManager<Deck>
{
public:
	DeckManager();
	~DeckManager();
};