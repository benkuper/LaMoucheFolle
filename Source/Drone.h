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

	enum DroneState {DISCONNECTED, CONNECTING, STABILIZING, READY , ERROR };
		
	IntParameter * targetRadio;
	IntParameter * channel;
	EnumParameter * speed;
	StringParameter * address;

	EnumParameter * droneState;

	Trigger * connectTrigger;
	Trigger * logParamsTOC;
	Trigger * logLogsTOC;
	Trigger * taskDump;
	
	Trigger * resetKalmanTrigger;
	Trigger * launchTrigger;
	Trigger * stopTrigger;
	Trigger * syncTrigger;

	EnumParameter * lightMode;
	ColorParameter * color;
	BoolParameter * headlight;

	BoolParameter * autoReconnect;
	BoolParameter * autoKillUpsideDown;

	Point3DParameter * targetPosition;
	Point3DParameter * realPosition;
	Point3DParameter * orientation;

	FloatParameter * yaw;
	BoolParameter * absoluteMode;


	FloatParameter * linkQuality;
	FloatParameter * voltage;
	BoolParameter * charging;
	BoolParameter * lowBattery;

	BoolParameter * initPIDSettings;
	BoolParameter * initAnchorPos;

	BoolParameter * enableLogConsole;
	BoolParameter * enableLogParams;

	ScopedPointer<Crazyflie> cf;

	Trigger * inTrigger;
	Trigger * outTrigger;

	String consoleBuffer;

	uint32 lastAckTime;
	uint32 ackTimeout;

	//Setup chronology
	bool droneHasStarted;
	bool droneHasFinishedInit;

	const float lowBatteryTimeCheck = 1.0f; //1 second below threshold to declare low battery, this allows fast voltage drops and raise (like when flying up from ground)
	float timeAtBelowLowBattery;

	//log blocks
	struct dataLog
	{
		float battery;
		uint8 charging;
		float x;
		float y;
		float z;
	} __attribute__((packed));

	struct feedbackLog
	{
		float pitch;
		float yaw;
		float roll;
	} __attribute__((packed));

	ScopedPointer<LogBlock<dataLog>> dataLogBlock;
	ScopedPointer<LogBlock<feedbackLog>> feedbackBlock;

	void launchCFThread();
	void stopCFThread();

	
	void onContainerParameterChangedAsync(Parameter * p, const var &) override;
	void onContainerParameterChangedInternal(Parameter * p) override;
	void onContainerTriggerTriggered(Trigger * t) override;

	bool setupCF();
	void selfTestCheck();

	template<class T>
	bool setParam(String group, String paramID, T value);
	bool setTargetPosition(float x, float y, float z, float yaw, bool showTrigger = true);
	bool setAnchors(Array<Vector3D<float>> positions);

	void logAllParams();
	void logAllLogs();

	void consoleCallback(const char * c);
	void emptyAckCallback(const crtpPlatformRSSIAck * a);
	void linkQualityCallback(float val);
	void dataLogCallback(uint32_t /*time*/, dataLog * data);
	void feedbackLogCallback(uint32_t /*time*/, feedbackLog * data);

	virtual void run() override;

	String getRadioString() const { return "radio://" + String(targetRadio->intValue()) + "/" + String(channel->intValue()) + "/" + speed->getValueData().toString() + "/" + address->stringValue(); }
	String getTypeString() const override { return "Drone"; }


private:
	SpinLock cfLock;
};

#ifdef _WIN32
#pragma pack(pop)
#endif