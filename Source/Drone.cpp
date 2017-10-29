/*
  ==============================================================================

    Drone.cpp
    Created: 19 Oct 2017 7:33:14pm
    Author:  Ben

  ==============================================================================
*/

#include "Drone.h"

Drone::Drone() :
	BaseItem("Drone"),
	cf(nullptr)
{
	targetRadio = addIntParameter("Radio","Target Radio to connect",0,0,16);
	channel = addIntParameter("Channel", "Target channel of the drone", 40, 0, 200);
	speed = addEnumParameter("Baudrate", "Speed of the connection");
	speed->addOption("250K", Crazyradio::Datarate_250KPS)->addOption("1M", Crazyradio::Datarate_1MPS)->addOption("2M", Crazyradio::Datarate_2MPS);

	address = addStringParameter("Address", "Address of the drone, without the 0x", "E7E7E7E7E7");

	realPosition = addPoint3DParameter("Position", "Position feedback from the drone");
	realPosition->setBounds(-10, 0, -10, 10, 10, 10);

	initTrigger = addTrigger("Init", "Init the connection to the radio");
	armTrigger = addTrigger("Arm", "Disable the thrust");

	linkQuality = addFloatParameter("Link Quality", "Quality of radio communication with the drone", 0, 0, 1);
	linkQuality->isEditable = false;

	inTrigger = addTrigger("Activity IN", "If received any communication from the drone");
	outTrigger = addTrigger("Activity OUT", "If any data has been sent to the drone");
	inTrigger->hideInEditor = true;
	inTrigger->isEditable = false;
	outTrigger->isEditable = false;
	outTrigger->hideInEditor = true;
}

Drone::~Drone()
{	
}

void Drone::setupCF()
{
	try
	{

		if (cf != nullptr)
		{
			//cf->alloff();
			cf = nullptr;
			
		}

	} catch (std::runtime_error e)
	{
		NLOG("Drone", "Error releasing drone");
	}
	
	try
	{
		NLOG("Drone","Init " << getRadioString() << "...");
		cf = new Crazyflie(targetRadio->intValue(),channel->intValue(),(Crazyradio::Datarate)(int)(speed->getValueData()),address->stringValue());
		LOG("Crazyflie is created.");

		std::function<void(const char *)> consoleF = std::bind(&Drone::consoleCallback, this, std::placeholders::_1);
		cf->setConsoleCallback(consoleF);

		std::function<void(const crtpPlatformRSSIAck *)> ackF = std::bind(&Drone::emptyAckCallback, this, std::placeholders::_1);
		cf->setEmptyAckCallback(ackF);

		std::function<void(float)> linkF = std::bind(&Drone::linkQualityCallback, this, std::placeholders::_1);
		cf->setLinkQualityCallback(linkF);


		startTimer(100);
		LOG("Reset Log");
		cf->logReset();
		cf->requestLogToc();
		cf->requestParamToc();

		NLOG("Drone", "Drone " << getRadioString() << " is Init !");

	} catch (std::runtime_error e)
	{
		NLOG("Drone", "Init failed " << e.what());
	}

}

void Drone::consoleCallback(const char * c)
{
	NLOG("Drone COnsole " + address->stringValue(),String(c));
}

void Drone::emptyAckCallback(const crtpPlatformRSSIAck * a)
{
	inTrigger->trigger();
}

void Drone::linkQualityCallback(float val)
{
	linkQuality->setValue(val);
}

void Drone::onContainerParameterChangedInternal(Parameter * p)
{
	if (p == realPosition)
	{
		if (cf != nullptr)
		{
			cf->sendExternalPositionUpdate(realPosition->x, realPosition->y, realPosition->z);
			outTrigger->trigger();
		}
	}
}

void Drone::onContainerTriggerTriggered(Trigger * t)
{
	if(t == initTrigger) setupCF();
	else if (t == armTrigger)
	{
		if (cf != nullptr)
		{
			try
			{
				//cf->trySysOff();

				
				LOG("Disable thrust");
				for (int i = 0; i<1; i++) cf->sendSetpoint(0, 0, 0, 0); //disable thrust lock, send more to be sure
				
				LOG("Disable fly with thrust 1000");
				for (int i = 0; i<1; i++) cf->sendSetpoint(0, 0, 0,1000); //disable thrust lock, send more to be sure
																		 //cf->setParam<int>(cf->getParamTocEntry("flightmode", "posSet")->id, 1);
				//cf->sendExternalPositionUpdate(realPosition->x, realPosition->z * 1000, realPosition->y * 1000);

				outTrigger->trigger();
			} catch (std::runtime_error)
			{
				LOG("Error sending :", e.what());
			}
			
		}
	}
}

void Drone::timerCallback()
{
	if (cf == nullptr) return;
	
	//DBG("Entry id : " << cf->getParamTocEntry("flightmode", "posSet")->id);
	//cf->sendSetpoint(0, 0, 0, 0); //disable thrust lock, send more to be sure
	//DBG("send " << realPosition->y);
	//outTrigger->trigger();
}