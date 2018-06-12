/*
  ==============================================================================

    RadialMenuItem.h
    Created: 12 Jun 2018 4:30:44pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class RadialMenuItem :
	public Component
{
public:
	RadialMenuItem(int menuID, const String &menuName, Image img);
	~RadialMenuItem();

	int menuID;
	String menuName;
	Image img;
};