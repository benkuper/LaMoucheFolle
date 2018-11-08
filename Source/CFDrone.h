/*
  ==============================================================================

    CFDrone.h
    Created: 19 Jun 2018 8:38:43am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#define LOG_POSITION_ID 0
#define LOG_POWER_ID 1
#define LOG_CALIB_ID 2

#define LOWBAT_ID		1000
#define LOWBAT_BLINK_ID 1001 
#define AUTOCONNECT_ID  1002
#define UPSIDE_DOWN_ID  1003

#include "CFCommand.h"
#include "CFParam.h"
#include "CFLog.h"
#include "CFPacket.h"




class CFDrone :
	public BaseItem,
	public MultiTimer
{

public:
	CFDrone();
	~CFDrone();

	
	IntParameter * droneId;

	enum DeckId { BCLEDRING, BCQI, BCBUZZER, BCBIGQUAD, BCDWM, BCUSD, BCZRANGER, BCFLOW, BCOA, BCMULTIRANGER, BCMOCAP, BCZRANGER2, BCFLOW2, DECKID_MAX };
	const String deckIds[DECKID_MAX] { "bcLedRing","bcQi","bcBuzzer","bcBigQuad","bcDWM1000","bcUSD","bcZRanger","bcFlow","bcOA","bcMultiranger","bcMocap","bcZRanger2","bcFlow2" };
	const String deckNames[DECKID_MAX] { "LED-Ring", "Qi charger", "Buzzer", "Big quad", "UWB LPS", "Micro - SD", "Z - Ranger", "Flow", "Obstacle Avoidance", "Multi - ranger", "Mocap marker deck", "Z - Ranger v2", "Flow V2" };

	enum LightMode { OFF, WHITE_SPINNER, COLOR_SPINNER, TILT_EFFECT, BRIGHTNESS, COLOR_SPINNER2, DOUBLE_SPINNER, SOLID_COLOR, FACTORY_TEST, BATTERY_STATUS, BOAT_LIGHTS, ALERT, GRAVITY, MEMORY, FADE_COLOR, LIGHTMODE_MAX};
	const String lightModeNames[LIGHTMODE_MAX] { "Off","White spinner","Color spinner","Tilt effect","Brightness","Color spinner2","Double spinner","Solid color","Factory test","Battery status","Boat lights","Alert","Gravity","Memory","Fade Color" };
	
	//Parameters
	enum DroneState { POWERED_OFF, DISCONNECTED, CONNECTING, CALIBRATING, ANALYSIS, TAKING_OFF, FLYING, LANDING, WARNING, READY, ERROR };

	enum TimerId { TIMER_PING, TIMER_TAKEOFF, TIMER_FLYING, TIMER_LANDING, TIMER_CALIBRATION, TIMER_MAX };
	const float timerFreqs[TIMER_MAX]{1, 20, 50, 20, 30 };

	enum MemoryAddress { ANCHOR_LIST = 0, ACTIVE_ANCHOR_LIST = 0x1000, ANCHOR_DATA = 0x2000 };

	ControllableContainer radioCC;
	BoolParameter * autoRadio;
	BoolParameter * autoChannel;
	IntParameter * targetRadio;
	IntParameter * channel;
	EnumParameter * baudRate;
	StringParameter * address;

	ControllableContainer controlsCC;
	
	Trigger * connectTrigger;
	Trigger * tocTrigger;
	Trigger * calibrateTrigger;
	Trigger * analyzeTrigger;
	Trigger * takeOffTrigger;
	FloatParameter * takeOffHeight;
	Trigger * landTrigger;
	Trigger * stopTrigger;
	Trigger * rebootTrigger;
	Trigger * setupNodesTrigger;
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

	//Parameters
	CFParamToc * paramToc;
	int currentParamRequestId; //to keep track when gathering all the parameters

	//Logs
	CFLogToc * logToc;
	int currentLogVariableId; //to keep track when gathering all the log variables

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

	//Calibration
	uint64 timeAtStartCalib;
	const uint64 calibTimeout = 3000; //3s of no stab is warning
	const float minConvergeDist = .005f;
	const uint64 minConvergeTime = 1500; //ms
	uint64 timeAtStartConverge;

	//Procedures
	uint64 timeAtStartTakeOff;
	uint64 timeAtStartLanding;
	float landingTime;

	//Physics
	double lastPhysicsUpdateTime;

	//Commands
	Array<CFCommand *, CriticalSection> commandQueue;

	void addCommand(CFCommand * command);
	CFCommand * getDefaultCommand();

	void addConnectionCommands();

	void addTakeoffCommand();
	void addFlyingCommand();
	void addLandingCommand();
	void addParamCommand(String paramName, var value);
	void addGetParamValueCommand(int  paramId);
	void addGetParamValueCommand(String paramName);
	void addRebootCommand();
	void addSetupNodesCommands();

	void syncToRealPosition();

	void updateControls();

	//Events
	void onContainerParameterChangedInternal(Parameter * p) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable *c) override;


	//Callbacks from CFRadioManager
	virtual void noAckReceived();
	virtual void packetReceived(const CFPacket &packet);


	virtual void consolePacketReceived(const String &msg);
	virtual void rssiAckReceived(uint8_t rssi);
	virtual void paramTocReceived(int crc, int size);
	virtual void paramInfoReceived(int id, String group, String name, int type, bool readOnly, int length, int sign);
	virtual void paramValueReceived(int id, var value);
	virtual void logTocReceived(int crc, int size);
	virtual void logVariableInfoReceived(String group, String name, int type);
	virtual void logBlockReceived(int blockId, var data);

	void updateQualityPackets(bool val);


	void stateChanged();

	void stopAllTimers();
	void startTimer(TimerId id);
	virtual void timerCallback(int timerID) override;

	//
	WeakReference<CFDrone>::Master masterReference;
};


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
