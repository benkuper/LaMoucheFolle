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

#if _WIN32
#define __attribute__(x) 
#pragma pack(push,1)
#endif

class Drone :
	public BaseItem,
	public Thread //thread the cf init to avoid blocking UI
{
public:
	Drone();
	~Drone();

	enum DroneState {DISCONNECTED, CONNECTING, READY , ERROR };
		
	IntParameter * targetRadio;
	IntParameter * channel;
	EnumParameter * speed;
	StringParameter * address;

	EnumParameter * droneState;

	Trigger * connectTrigger;
	Trigger * logParams;
	Trigger * logLogs;
	Trigger * taskDump;
	Trigger * resetKalmanTrigger;

	EnumParameter * lightMode;
	ColorParameter * color;
	BoolParameter * headlight;

	BoolParameter * autoReconnect;

	Point3DParameter * targetPosition;
	Point3DParameter * realPosition;


	FloatParameter * linkQuality;
	FloatParameter * voltage;
	BoolParameter * charging;
	BoolParameter * lowBattery;

	ScopedPointer<Crazyflie> cf;

	Trigger * inTrigger;
	Trigger * outTrigger;

	String consoleBuffer;

	uint32 lastAckTime;
	uint32 ackTimeout;

	//Setup chronology
	bool droneHasStarted;
	bool droneHasFinishedInit;

	//log blocks
	struct dataLog
	{
		float battery;
		uint8 charging;
		float x;
		float y;
		float z;
	} __attribute__((packed));

	ScopedPointer<LogBlock<dataLog>> dataLogBlock;

	void launchCFThread();
	void stopCFThread();

	void onContainerParameterChangedInternal(Parameter * p) override;
	void onContainerTriggerTriggered(Trigger * t) override;

	bool setupCF();
	void selfTestCheck();

	template<class T>
	bool setParam(String group, String paramID, T value);
	bool setTargetPosition(float x, float y, float z, bool showTrigger = true);
	bool setAnchors(Array<Point3DParameter *> positions);

	void logAllParams();
	void logAllLogs();

	void consoleCallback(const char * c);
	void emptyAckCallback(const crtpPlatformRSSIAck * a);
	void linkQualityCallback(float val);
	void dataLogCallback(uint32_t /*time*/, dataLog * data);

	virtual void run() override;

	String getRadioString() const { return "radio://" + String(targetRadio->intValue()) + "/" + String(channel->intValue()) + "/" + speed->getValueData().toString() + "/" + address->stringValue(); }
	String getTypeString() const override { return "Drone"; }


private:
	SpinLock cfLock;
};

#ifdef _WIN32
#pragma pack(pop)
#endif