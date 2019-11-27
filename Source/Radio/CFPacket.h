/*
  ==============================================================================

	CFPacket.h
	Created: 19 Jun 2018 1:55:30pm
	Author:  Ben

  ==============================================================================
*/


#pragma once

#include "JuceHeader.h"
#include "crtp.h"

class Drone;

class CFPacket
{
public:
	enum Type { UNKNOWN, CONSOLE, LOG_TOC_INFO, LOG_TOC_ITEM, LOG_CONTROL, LOG_DATA, PARAM_TOC_INFO, PARAM_TOC_ITEM, MEMORY_NUMBER, MEMORY_INFO, MEMORY_READ, MEMORY_WRITE, PARAM_VALUE, RSSI_ACK, LPP_SHORT_PACKET, TYPES_MAX };
	const String typeStrings[TYPES_MAX]{ "Unknown", "Console", "LogTocInfo", "LogItem", "LogControl", "LogData", "ParamTocInfo", "ParamTocItem", "MemoryNumber", "MemoryInfo", "MemoryRead", "MemoryWrite", "ParamValue", "RSSI", "LPPSHortPacket"};
	
	CFPacket(Drone * drone, const ITransport::Ack &ack); 
	~CFPacket();

	static const uint8_t safeLinkPacket[3];

	Type type;
	var data; 

	bool hasSafeLink;

	String getTypeString() const { return typeStrings[type]; }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CFPacket)
};