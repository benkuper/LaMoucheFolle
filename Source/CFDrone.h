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

	FloatParameter * linkQuality;
	const int zeroQualityTimeout = 1000; //ms after linkQuality has been to 0 to mark as disconnected

	//CFLogToc * logToc;

	Array<CFCommand *, CriticalSection> commandQueue;

	void addCommand(CFCommand * command);

	//Callbacks from CFRadioManager
	virtual void noAckReceived();
	virtual void consolePacketReceived(String data);
	virtual void rssiAckReceived(uint8_t rssi);
	virtual void paramTocReceived(CFParamToc * toc);
	virtual void paramInfoReceived(CFParam * param);
	virtual void paramValueReceived(CFParam * param);
	/*
	virtual void logTocReceived(CFLogToc * toc) = 0;
	virtual void logBlockReceived(CFLogBlock * block) = 0;
	*/
	virtual void genericPacketReceived(const CFPacket &packet);


	WeakReference<CFDrone>::Master masterReference;
};