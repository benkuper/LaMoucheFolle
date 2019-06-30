/*
  ==============================================================================

    CFDrone.h
    Created: 19 Jun 2018 8:38:43am
    Author:  Ben

  ==============================================================================
*/

#pragma once


#include "JuceHeader.h"
#include "Crazyflie.h"

// BLOCK DATA HOLDERS
#if _WIN32
#define __attribute__(x) 
#pragma pack(push,1)
#endif

struct BatteryBlock
{
	uint8 battery;
	float voltage;
	uint8 charging;
} __attribute__((packed));

struct PosBlock
{
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
} __attribute__((packed));


struct CalibBlock
{
	float varianceX;
	float varianceY;
	float varianceZ;
} __attribute__((packed));


#ifdef _WIN32
#pragma pack(pop)
#endif



class CFDrone :
	public BaseItem,
	public Thread
{

public:
	CFDrone();
	~CFDrone();

	void clearItem() override;

	enum DeckId { BCLEDRING, BCQI, BCBUZZER, BCBIGQUAD, BCDWM, BCUSD, BCZRANGER, BCFLOW, BCOA, BCMULTIRANGER, BCMOCAP, BCZRANGER2, BCFLOW2, BCRRZR, BCGTGPS, BCCPPM, LIGHTHOUSE, DECKID_MAX };
	const String deckIds[DECKID_MAX] { "bcLedRing","bcQi","bcBuzzer","bcBigQuad","bcDWM1000","bcUSD","bcZRanger","bcFlow","bcOA","bcMultiranger","bcMocap","bcZRanger2","bcFlow2", "bcRZR", "bcGTGPS", "bcCPPM", "bcLighthouse4" };
	const String deckNames[DECKID_MAX] { "LED-Ring", "Qi charger", "Buzzer", "Big quad", "UWB LPS", "Micro - SD", "Z - Ranger", "Flow", "Obstacle Avoidance", "Multi - ranger", "Mocap marker deck", "Z - Ranger v2", "Flow V2", "RZR", "GTGPS", "CPPM", "Lighthouse"};

	enum LightMode { OFF, WHITE_SPINNER, COLOR_SPINNER, TILT_EFFECT, BRIGHTNESS, COLOR_SPINNER2, DOUBLE_SPINNER, SOLID_COLOR, FACTORY_TEST, BATTERY_STATUS, BOAT_LIGHTS, ALERT, GRAVITY, MEMORY, FADE_COLOR, LIGHTMODE_MAX};
	const String lightModeNames[LIGHTMODE_MAX] { "Off","White spinner","Color spinner","Tilt effect","Brightness","Color spinner2","Double spinner","Solid color","Factory test","Battery status","Boat lights","Alert","Gravity","Memory","Fade Color" };
	
	//Parameters
	enum DroneState { DISCONNECTED, CALIBRATING, HEALTH_CHECK, TAKING_OFF, FLYING, LANDING, WARNING, READY, ERROR };

	/*
	enum TimerId { TIMER_PING, TIMER_TAKEOFF, TIMER_FLYING, TIMER_LANDING, TIMER_CALIBRATION, TIMER_MAX };
	const float timerFreqs[TIMER_MAX]{100, 20, 50, 20, 30 };
	*/

	//enum MemoryAddress { ANCHOR_LIST = 0, ACTIVE_ANCHOR_LIST = 0x1000, ANCHOR_DATA = 0x2000 };

	IntParameter* droneId;

	std::unique_ptr<Crazyflie> cf;
	SpinLock droneLock;
	ControllableContainer controlsCC;
	
	Trigger * calibrateTrigger;
	Trigger * takeOffTrigger;
	Trigger * landTrigger;
	Trigger * stopTrigger;
	Trigger * rebootTrigger;
	Trigger * propCheckTrigger;

	ControllableContainer statusCC;
	EnumParameter * state;
	
	FloatParameter * linkQuality;
	FloatParameter * batteryLevel;
	FloatParameter * voltage;
	BoolParameter * charging;
	BoolParameter * lowBattery;
	
	BoolParameter * selfTestProblem;
	BoolParameter * batteryProblem;
	
	ControllableContainer decksCC;
	HashMap<String, BoolParameter *> deckMap;
	ControllableContainer wingsCC;

	ControllableContainer flightCC;
	Point3DParameter * desiredPosition;
	Point3DParameter * desiredSpeed;
	Point3DParameter * desiredAcceleration;
	Point3DParameter * targetPosition;
	Point3DParameter * targetSpeed;
	Point3DParameter * targetAcceleration;
	Point3DParameter * realPosition;
	Point3DParameter * positionDiff;
	FloatParameter * positionNoise;

	//testing
	double lastRealPosTime;
	Vector3D<float> lastRealPos;

	FloatParameter * targetYaw;
	Point3DParameter * orientation;
	BoolParameter * upsideDown;

	ControllableContainer lightingCC;
	EnumParameter * lightMode;
	ColorParameter * color;
	FloatParameter * fadeTime;
	BoolParameter * headlight;
	BoolParameter * stealthMode;


	//Analysis and Calib feedfback
	FloatParameter * calibrationProgress;
	FloatParameter * analysisProgress;
	
	//Ping
	uint32 lastPingTime;

	//Radio
	bool safeLinkActive;
	bool safeLinkUpFlag;
	bool safeLinkDownFlag;

	//Link quality
	static const int maxQualityPackets = 100;
	bool qualityPackets[maxQualityPackets];
	int qualityPacketIndex;
	const int zeroQualityTimeout = 1000; //ms after linkQuality has been to 0 to mark as disconnected

	//Console
	String consoleBuffer;
	String lastMessage;

	//Calibration
	bool isCalibrated;
	double timeAtStartCalib;
	const double calibTimeout = 3; //3s of no stab is warning
	const float minConvergeDist = .005f;
	const double minConvergeTime = 1.5; //ms
	double timeAtStartConverge;

	//Procedures
	double timeAtStartTakeOff;
	double timeAtStartLanding;
	double timeAtStartHealthCheck;
	float landingTime;

	//Physics
	double lastPhysicsUpdateTime;

	std::function<void(const char *)> consoleFunc;
	std::function<void(const crtpPlatformRSSIAck * ack)> emptyAckFunc;
	std::function<void(float)> linkQualityFunc;

	std::unique_ptr<LogBlock<BatteryBlock>> batteryBlock;
	std::unique_ptr<LogBlock<PosBlock>> posBlock;
	std::unique_ptr<LogBlock<CalibBlock>> calibBlock;

	std::function<void(uint32, BatteryBlock*)> batteryBlockCallback;
	std::function<void(uint32, PosBlock*)> posBlockCallback;
	std::function<void(uint32, CalibBlock*)> calibBlockCallback;


	//Commands
	
	void setupDrone();
	void setParam(String group, String name, var value);

	void syncToRealPosition();
	void updateControls();

	//Events
	void onContainerParameterChangedInternal(Parameter * p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable *c) override;

	void stateChanged();

	virtual void consolePacketReceived(const char *);
	virtual void rssiAckReceived(const crtpPlatformRSSIAck * ack);
	virtual void linkQualityReceived(float quality);

	virtual void batteryBlockReceived(uint32, BatteryBlock*);
	virtual void posBlockReceived(uint32, PosBlock*);
	virtual void calibBlockReceived(uint32, CalibBlock*);

	virtual void run() override;

	void runConnect();
	void runCalibrate();
	void runHealthCheck();
	void runFlying();

	String getURI() const;

	WeakReference<CFDrone>::Master masterReference;

};

