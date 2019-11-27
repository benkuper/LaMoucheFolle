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
#include "Drone/Drone.h"

class CFRadioManager :
	public Thread,
	public Timer
{
public :
	juce_DeclareSingleton(CFRadioManager, true)
	CFRadioManager();
	~CFRadioManager();

	OwnedArray<Crazyradio> radios;
	int numRadios;

	bool shouldCheckRadios;
	
	const uint32_t radioCheckTime = 1000;

	//Stats
	int packetsPerSeconds;
	
	void run() override;

	void timerCallback() override;
	//
	void setupRadios();
	bool processAck(CFCommand * command, ITransport::Ack &ack);	
};