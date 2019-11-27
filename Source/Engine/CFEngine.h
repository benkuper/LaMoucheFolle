/*
==============================================================================

CFEngine.h
Created: 2 Apr 2016 11:03:21am
Author:  Martin Hermant

==============================================================================
*/

#pragma once

#include "JuceHeader.h"

class CFEngine :
	public Engine
{
public:
	CFEngine();
	~CFEngine();

	void clearInternal() override;

	var getJSONData() override;
	void loadJSONDataInternalEngine(var data, ProgressTask * loadingTask) override;


	String getMinimumRequiredFileVersion() override;
};

