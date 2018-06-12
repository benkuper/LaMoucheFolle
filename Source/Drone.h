/*
  ==============================================================================

    Drone.h
    Created: 12 Jun 2018 4:34:07pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Crazyflie.h"
#include "JuceHeader.h"

#define MAX_DECKS 3 //max supported decks in CF2

#if _WIN32
#define __attribute__(x) 
#pragma pack(push,1)
#endif

struct BatteryLog
{
	float battery;
	uint8 lowBattery;
	uint8 charging;
} __attribute__((packed));

struct PosLog
{
	float x;
	float y;
	float z;
} __attribute__((packed));


#ifdef _WIN32
#pragma pack(pop)
#endif



class Drone :
	public BaseItem,
	public Thread //thread the cf init to avoid blocking UI
{
public:
	Drone();
	~Drone();

	enum DroneState { POWERED_OFF, DISCONNECTED, CONNECTING, CALIBRATING, ANALYSIS , WARNING, READY, ERROR };
	
	ScopedPointer<Crazyflie> cf;

	ControllableContainer radioCC;
	BoolParameter * autoRadio;
	BoolParameter * autoChannel;
	IntParameter * targetRadio;
	IntParameter * channel;
	EnumParameter * baudRate;
	StringParameter * address;

	ControllableContainer controlsCC;
	Trigger * connectTrigger;
	Trigger * calibrateTrigger;
	Trigger * takeOffTrigger;
	Trigger * landTrigger;
	Trigger * rebootTrigger;

	ControllableContainer statusCC;
	EnumParameter * state;
	FloatParameter * linkQuality;
	FloatParameter * batteryLevel;
	BoolParameter * charging;
	FloatParameter * chargingProgress;
	BoolParameter * lowBattery;
	BoolParameter * selfTestProblem;
	BoolParameter * batteryProblem;
	ControllableContainer decksCC;
	ControllableContainer wingsCC;

	ControllableContainer flightCC;
	Point3DParameter * realPosition;
	Point3DParameter * targetPosition;
	FloatParameter * yaw;

	ControllableContainer lightingCC;
	EnumParameter * lightMode;
	ColorParameter * color;
	FloatParameter * fadeTime;
	BoolParameter * headlight;
	BoolParameter * stealthMode;

	//Calibration
	float calibrationProgress;
	Vector3D<float> lastRealPos;
	const float minConvergeDist = .2f;
	float timeAtStartConverge;
	const float minConvergeTime = 1.0f;

	//Analysis
	float analysisProgress;

	//Procedures
	bool isLaunching;
	bool isLanding;
	float timeAtStartTakeOff;
	float timeAtStartLanding;

	
	void onContainerParameterChangedInternal(Parameter * p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable *c) override;

	//Callbacks
	void consoleCallback(const char * c);
	void emptyAckCallback(const crtpPlatformRSSIAck * a);
	void linkQualityCallback(float val);
	
	//Logging
	String consoleBuffer;
	ScopedPointer<LogBlock<BatteryLog>> batteryLogBlock;
	ScopedPointer<LogBlock<PosLog>> posLogBlock;
	void batteryLogCallback(uint32_t, BatteryLog * data);
	void posLogCallback(uint32_t, PosLog * data);

	//Helper
	bool allDecksAreConnected();

	//Thread
	void run() override;

	void stateUpdated();
	
	void ping();
	void connect();
	void calibrate();
	void processCalibration();

	void takeOff();
	void updateTakeOff();
	void updateFlyingPosition();
	void land();

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	//CF Control
	SpinLock paramLock;

	template<class T>
	bool setParam(String group, String paramID, T value);

};


