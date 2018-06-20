/*
  ==============================================================================

    CFDrone.h
    Created: 19 Jun 2018 8:38:43am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "CFCommand.h"
#include "CFParam.h"
#include "CFLog.h"
#include "CFPacket.h"

class CFDrone :
	public BaseItem
{
public:
	CFDrone();
	~CFDrone();

	CFParamToc * paramToc;
	IntParameter * droneId;

	//Radio
	bool safeLinkActive;
	bool safeLinkUpFlag;
	bool safeLinkDownFlag;

	FloatParameter * linkQuality;
	static const int maxQualityPackets = 100;
	bool qualityPackets[maxQualityPackets];
	int qualityPacketIndex;
	const int zeroQualityTimeout = 1000; //ms after linkQuality has been to 0 to mark as disconnected

	String consoleBuffer;

	//CFLogToc * logToc;

	Array<CFCommand *, CriticalSection> commandQueue;

	void addCommand(CFCommand * command);

	//Callbacks from CFRadioManager
	virtual void noAckReceived();
	virtual void packetReceived(const CFPacket &packet);


	virtual void consolePacketReceived(const String &msg);
	virtual void rssiAckReceived(uint8_t rssi);
	virtual void paramTocReceived(CFParamToc * toc);
	virtual void paramInfoReceived(CFParam * param);
	virtual void paramValueReceived(CFParam * param);
	/*
	virtual void logTocReceived(CFLogToc * toc) = 0;
	virtual void logBlockReceived(CFLogBlock * block) = 0;
	*/

	virtual void safeLinkReceived();

	void updateQualityPackets(bool val);

	WeakReference<CFDrone>::Master masterReference;
};