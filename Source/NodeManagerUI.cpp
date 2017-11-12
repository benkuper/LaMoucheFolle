/*
  ==============================================================================

    NodeManagerUI.cpp
    Created: 10 Nov 2017 10:29:44am
    Author:  Ben

  ==============================================================================
*/

#include "NodeManagerUI.h"

NodeManagerUI::NodeManagerUI(const String & name, NodeManager * manager) :
	BaseManagerShapeShifterUI(name, manager)
{
	addExistingItems();
}

NodeManagerUI::~NodeManagerUI()
{
}
