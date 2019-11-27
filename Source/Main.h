
/*
==============================================================================

This file was auto-generated!

It contains the basic startup code for a Juce application.

==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#pragma warning(disable:4244 4100 4305)

#include "MainComponent.h"
#include "Engine/CFEngine.h"
#include "Engine/CFSettings.h"


//==============================================================================
class CFApplication : public OrganicApplication
{
public:
	//==============================================================================
	CFApplication() : OrganicApplication("LaMoucheFolle"){}
	~CFApplication();

	//==============================================================================
	void initialiseInternal(const String& /*commandLine*/) override;
};



//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(CFApplication)
