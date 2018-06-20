/*
  ==============================================================================

    CFDrone.cpp
    Created: 19 Jun 2018 8:38:43am
    Author:  Ben

  ==============================================================================
*/

#include "CFDrone.h"

CFDrone::CFDrone() : 
	BaseItem("CFDrone"),
	paramToc(nullptr)
{
	droneId = addIntParameter("ID", "Id of the drone 0 will be adress 0xE7E7E7E700 and channel 0, 1 will be 0xE7E7E7E701 and channel 2", 0, 0, 60);
	linkQuality = addFloatParameter("Link quality", "Quality of the link based on received vs lost packet", 0, 0, 1);
}

CFDrone::~CFDrone() 
{
	masterReference.clear();
}

void CFDrone::addCommand(CFCommand * command)
{
	if (!enabled->boolValue()) return;

	commandQueue.getLock().enter();
	commandQueue.add(command); 
	commandQueue.getLock().exit();
}


void CFDrone::noAckReceived()
{
	//DBG("No ack received");
	linkQuality->setValue(linkQuality->floatValue() - .01f);
}

void CFDrone::consolePacketReceived(String data)
{
	linkQuality->setValue(linkQuality->floatValue() + .01f);
}

void CFDrone::rssiAckReceived(uint8_t rssi)
{
	linkQuality->setValue(linkQuality->floatValue() + .01f);
}

void CFDrone::paramTocReceived(CFParamToc * toc)
{
	linkQuality->setValue(linkQuality->floatValue() + .01f);
}

void CFDrone::paramInfoReceived(CFParam * param)
{
	linkQuality->setValue(linkQuality->floatValue() + .01f);
}

void CFDrone::paramValueReceived(CFParam * param)
{
	linkQuality->setValue(linkQuality->floatValue() + .01f);
}

void CFDrone::genericPacketReceived(const CFPacket &packet)
{
	linkQuality->setValue(linkQuality->floatValue() + .01f);
}
