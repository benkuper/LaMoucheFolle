/*
  ==============================================================================

    OSCOutput.h
    Created: 19 Oct 2017 7:35:43pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Controller.h"

class OSCController :
	public Controller
{
public:
	OSCController(var params);
	~OSCController();

	String getTypeString() const override { return "OSC"; }
	static OSCController * create(var params) { return new OSCController(params); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCController)
};