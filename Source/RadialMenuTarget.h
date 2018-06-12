/*
  ==============================================================================

    RadialMenuTarget.h
    Created: 12 Jun 2018 4:30:32pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "RadialMenuItem.h"

class RadialMenuTarget
{
public:
	RadialMenuTarget(Component * targetComponent);
	~RadialMenuTarget();

	Component * targetComponent;
	Array<RadialMenuItem *> getItems;
};