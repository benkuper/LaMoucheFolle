/*
  ==============================================================================

    DeckManager.cpp
    Created: 29 Nov 2019 10:56:29am
    Author:  bkupe

  ==============================================================================
*/

#include "DeckManager.h"

DeckManager::DeckManager() :
	BaseManager("Decks")
{
	selectItemWhenCreated = false;
}

DeckManager::~DeckManager()
{
}
