/*
  ==============================================================================

    Drone2.h
    Created: 12 May 2018 5:18:44pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Crazyflie.h"
#include "JuceHeader.h"


#if _WIN32
#define __attribute__(x) 
#pragma pack(push,1)
#endif


class Drone2 :
	public BaseItem,
	public Thread //thread the cf init to avoid blocking UI
{
public:
	Drone2();
	~Drone2();

	enum DroneState { DISCONNECTED, CONNECTING, CALIBRATING, READY, TAKEOFF, FLYING, LANDING, ERROR, BOOTING };
	
	ScopedPointer<Crazyflie> cf;

	IntParameter * targetRadio;
	IntParameter * channel;
	EnumParameter * speed;
	StringParameter * address;

	EnumParameter * state;



	Trigger * connectTrigger;
	Trigger * calibrateTrigger;
	Trigger * takeOffTrigger;
	Trigger * unlockTrigger;
	Trigger * landTrigger;
	Trigger * rebootTrigger;

	Point3DParameter * targetPosition;


	//Procedure variables
	DroneState lastState;
	float timeAtStartTakeOff;
	float timeAtStartLanding;

	//Calibration
	Vector3D<float> lastRealPos;
	const float minConvergeDist = .2f;
	float timeAtStartConverge;
	const float minConvergeTime = 1.0f;

	//Light
	EnumParameter * lightMode;
	ColorParameter * color;
	BoolParameter * headlight;
	int prevLightMode;
	Colour prevColor;
	bool prevHeadLight;

	//Battery
	BoolParameter * lowBattery;
	const float lowBatteryTimeCheck = 1.0f; //1 second below threshold to declare low battery, this allows fast voltage drops and raise (like when flying up from ground)
	float timeAtBelowLowBattery;

	// Feedback
	Point3DParameter * realPosition;
	FloatParameter * linkQuality;
	FloatParameter * voltage;
	
	//Connection
	const uint32 ackTimeout = 3000;
	uint32 lastAckTime;

	//Logging
	String consoleBuffer;


	void onContainerParameterChangedInternal(Parameter * p) override;
	void onContainerTriggerTriggered(Trigger * t) override;


	void run() override;
	
	//Threaded functions
	void setState(DroneState s);

	void connect();
	void checkConnection();
	void calibrate();
	void processCalibration();

	void takeOff();
	void updateTakeOff();
	void updateFlyingPosition();
	void land();

	void checkBattery();

	template<class T>
	bool setParam(String group, String paramID, T value);




	//Logging
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

	void consoleCallback(const char * c);
	void emptyAckCallback(const crtpPlatformRSSIAck * a);
	void linkQualityCallback(float val);
	void dataLogCallback(uint32_t /*time*/, dataLog * data);
	void feedbackLogCallback(uint32_t /*time*/, feedbackLog * data);




};


#ifdef _WIN32
#pragma pack(pop)
#endif