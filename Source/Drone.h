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


struct CalibLog
{
	float varianceX;
	float varianceY;
	float varianceZ;
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

	enum DroneState { POWERED_OFF, DISCONNECTED, CONNECTING, CALIBRATING, ANALYSIS , TAKING_OFF, FLYING, LANDING, WARNING, READY, ERROR };
	
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
	Trigger * analyzeTrigger;
	Trigger * takeOffTrigger;
	Trigger * landTrigger;
	Trigger * stopTrigger;
	Trigger * rebootTrigger;

	ControllableContainer statusCC;
	EnumParameter * state;
	FloatParameter * linkQuality;
	FloatParameter * batteryLevel;
	BoolParameter * charging;
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

	//Analysis and Calib feedfback
	FloatParameter * calibrationProgress;
	FloatParameter * analysisProgress;

	//Packet timing
	const uint32 pingTime = 300; //ms between pings
	uint32 lastPingTime;
	const int maxNoPongCount = 3;
	int noPongCount;

	//Calibration
	float timeAtStartCalib;
	const float calibTimeout = 5.0f;
	const float minConvergeDist = .005f;
	const uint32 minConvergeTime = 2000; //ms
	uint64 timeAtStartConverge;

	//Procedures
	float timeAtStartTakeOff;
	float timeAtStartLanding;
	float landingTime;

	//Physics
	double lastTime;
	double deltaTime;
	Vector3D<float> lastTargetPosition;
	Vector3D<float> targetSpeed;
	Vector3D<float> lastSpeed;
	Vector3D<float> targetAcceleration;
	
	//Events
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
	ScopedPointer<LogBlock<CalibLog>> calibLog;
	void batteryLogCallback(uint32_t, BatteryLog * data);
	void posLogCallback(uint32_t, PosLog * data);
	void calibLogCallback(uint32_t, CalibLog * data);

	//Helper
	bool allDecksAreConnected();
	bool droneHasLogVariable(String group, String name);

	//Thread
	void run() override;

	void stateUpdated();
	
	void setupCF();
	void ping();
	void connect();
	void calibrate();
	void processCalibration();

	void takeOff();
	void updateTakeOff();
	void updateFlyingPosition();
	void land();
	void updateLanding();

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	//CF Control
	SpinLock paramLock;

	template<class T>
	bool setParam(String group, String paramID, T value, bool force = false);

};


