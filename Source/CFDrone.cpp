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
	paramToc(nullptr),
	safeLinkActive(false),
	safeLinkDownFlag(false),
	safeLinkUpFlag(false),
	qualityPacketIndex(0)
{
	memset(qualityPackets, 0, maxQualityPackets * sizeof(bool));

	droneId = addIntParameter("ID", "Id of the drone 0 will be adress 0xE7E7E7E700 and channel 0, 1 will be 0xE7E7E7E701 and channel 2", 0, 0, 60);
	linkQuality = addFloatParameter("Link quality", "Quality of the link based on received vs lost packet", 0, 0, 1);

	addCommand(CFCommand::createActivateSafeLink(this));
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
	updateQualityPackets(false);
}

void CFDrone::packetReceived(const CFPacket & packet)
{
	//DBG("Packet received " << packet.getTypeString());
	updateQualityPackets(true);

	switch (packet.type)
	{
	case CFPacket::CONSOLE: consolePacketReceived(packet.data.toString()); break;
	case CFPacket::RSSI_ACK: rssiAckReceived((int)packet.data); break;
	case CFPacket::SAFELINK: safeLinkReceived(); break;
	}
}

void CFDrone::consolePacketReceived(const String &msg)
{
	

	if (msg.containsChar('\n'))
	{
		consoleBuffer << msg.substring(0, msg.indexOfChar('\n'));
		
		String lowerCase = consoleBuffer.toLowerCase();
		if (lowerCase.contains("error") || lowerCase.contains("fail")) NLOGERROR(niceName, consoleBuffer);
		else if (lowerCase.contains("warning")) NLOGWARNING(niceName, consoleBuffer);
		else NLOG(niceName, consoleBuffer);
		consoleBuffer.clear();
	} else
	{
		String c = msg.substring(0, msg.indexOfChar((char)1));
		consoleBuffer << c;
		
	}
}

void CFDrone::rssiAckReceived(uint8_t)
{
	//DBG("RSSI Ack : " << rssi);
}

void CFDrone::paramTocReceived(CFParamToc * toc)
{
}

void CFDrone::paramInfoReceived(CFParam * param)
{
}

void CFDrone::paramValueReceived(CFParam * param)
{
}

void CFDrone::safeLinkReceived()
{
	NLOG(niceName, "Safe link activated");
	safeLinkActive = true;
}

void CFDrone::updateQualityPackets(bool val)
{
	bool oldVal = qualityPackets[qualityPacketIndex];
	if (oldVal != val)
	{
		linkQuality->setValue(linkQuality->floatValue() + (val ? .01f : -.01f));
		qualityPackets[qualityPacketIndex] = val;

		float p = 0;
		for (int i = 0; i < maxQualityPackets; i++)
		{
			p += (int)qualityPackets[i];
		}

	}
	qualityPacketIndex = (qualityPacketIndex + 1) % maxQualityPackets;

}
