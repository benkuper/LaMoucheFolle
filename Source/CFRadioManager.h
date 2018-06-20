/*
  ==============================================================================

    CFRadioManager.h
    Created: 19 Jun 2018 8:36:11am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "Crazyradio.h"
#include "CFCommand.h"
#include "CFPacket.h"
#include "CFDrone.h"

class CFRadioManager :
	public Thread
{
public :
	juce_DeclareSingleton(CFRadioManager, true)
	CFRadioManager();
	~CFRadioManager();

	OwnedArray<Crazyradio> radios;
	int numRadios;
	
	const uint32_t radioCheckTime = 1000;

	//Stats
	int packetsPerSeconds;
	
	void run();

	//
	void setupRadios();
	bool processAck(WeakReference<CFDrone> drone, ITransport::Ack &ack);	
};