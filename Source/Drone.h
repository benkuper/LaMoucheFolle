/*
  ==============================================================================

    Drone.h
    Created: 19 Oct 2017 7:33:14pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "Crazyflie.h"

class Drone :
	public BaseItem,
	public Timer
{
public:
	Drone();
	~Drone();
		
	IntParameter * targetRadio;
	IntParameter * channel;
	EnumParameter * speed;
	StringParameter * address;

	Trigger * initTrigger;
	Trigger * armTrigger;

	Point3DParameter * realPosition;

	FloatParameter * linkQuality;

	ScopedPointer<Crazyflie> cf;

	Trigger * inTrigger;
	Trigger * outTrigger;

	void setupCF();

	void consoleCallback(const char * c);
	void emptyAckCallback(const crtpPlatformRSSIAck * a);
	void linkQualityCallback(float val);

	void onContainerParameterChangedInternal(Parameter * p) override;
	void onContainerTriggerTriggered(Trigger * t) override;

	String getRadioString() const { return "radio://" + String(targetRadio->intValue()) + "/" + String(channel->intValue()) + "/" + speed->getValueData().toString() + "/" + address->stringValue();  }
	String getTypeString() const override { return "Drone"; }


	// Inherited via Timer
	virtual void timerCallback() override;

};