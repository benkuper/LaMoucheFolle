/*
  ==============================================================================

    CFDrone.h
    Created: 19 Jun 2018 8:38:43am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#define MAX_DECKS 3 //max supported decks in CF2


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

	CFParamToc * paramToc;
	IntParameter * droneId;

	enum DeckId {NONE, BCLEDRING, BCQI, BCBUZZER, BCBIGQUAD, BCDWM, BCUSD, BCZRANGER, BCFLOW, BCOA, BCMULTIRANGER, BCMOCAP, BCZRANGER2, BCFLOW2, DECKID_MAX };
	const String deckIds[DECKID_MAX] {"None", "bcLedRing","bcQi","bcBuzzer","bcBigQuad","bcDWM","bcUSD","bcZRanger","bcFlow","bcOA","bcMultiranger","bcMocap","bcZRanger2","bcFlow2" };
	const String deckNames[DECKID_MAX] {"None", "LED-Ring", "Qi charger", "Buzzer", "Big quad", "UWB LPS", "Micro - SD", "Z - Ranger", "Flow", "Obstacle Avoidance", "Multi - ranger", "Mocap marker deck", "Z - Ranger v2", "Flow V2" };

	enum LightMode { OFF, WHITE_SPINNER, COLOR_SPINNER, TILT_EFFECT, BRIGHTNESS, COLOR_SPINNER2, DOUBLE_SPINNER, SOLID_COLOR, FACTORY_TEST, BATTERY_STATUS, BOAT_LIGHTS, ALERT, GRAVITY, MEMORY, FADE_COLOR, LIGHTMODE_MAX};
	const String lightModeNames[LIGHTMODE_MAX] { "Off","White spinner","Color spinner","Tilt effect","Brightness","Color spinner2","Double spinner","Solid color","Factory test","Battery status","Boat lights","Alert","Gravity","Memory","Fade Color" };
	
	//Parameters
	enum DroneState { POWERED_OFF, DISCONNECTED, CONNECTING, CALIBRATING, ANALYSIS, TAKING_OFF, FLYING, LANDING, WARNING, READY, ERROR };
	enum TimerId { TIMER_PING, TIMER_TAKEOFF, TIMER_FLYING, TIMER_LANDING, TIMER_CALIBRATION, TIMER_MAX };
	const float timerFreqs[TIMER_MAX]{2, 20, 50, 20, 30 };

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
	Point3DParameter * desiredPosition;
	Point3DParameter * desiredSpeed;
	Point3DParameter * desiredAcceleration;
	Point3DParameter * targetPosition;
	Point3DParameter * targetSpeed;
	Point3DParameter * targetAcceleration;
	FloatParameter * yaw;
	Point3DParameter * realPosition;


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
	const uint32 pingTime = 500; //ms between pings
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
	const uint64 calibTimeout = 6000; //max 6s to calibrate
	const float minConvergeDist = .005f;
	const uint64 minConvergeTime = 1000; //ms
	uint64 timeAtStartConverge;

	//Procedures
	uint64 timeAtStartTakeOff;
	uint64 timeAtStartLanding;
	float landingTime;

	//Physics
	double lastPhysicsUpdateTime;

	//CFLogToc * logToc;

	Array<CFCommand *, CriticalSection> commandQueue;

	void addCommand(CFCommand * command);
	CFCommand * getDefaultCommand();

	void addConnectionCommands();

	void addTakeoffCommand();
	void addFlyingCommand();
	void addLandingCommand();
	void addParamCommand(String paramName, var value);
	void addRebootCommand();

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
	virtual void paramInfoReceived(int paramId, String group, String name, int type, bool readOnly, int length, int sign);
	virtual void paramValueReceived(CFParam * param);
	/*
	data.getDynamicObject()->setProperty("group", r->group);
	data.getDynamicObject()->setProperty("type", r->type);
	data.getDynamicObject()->setProperty("name", r->text);
	data.getDynamicObject()->setProperty("realOnly", r->readonly);
	data.getDynamicObject()->setProperty("length", r->length);
	data.getDynamicObject()->setProperty("sign", r->sign)


	virtual void logTocReceived(CFLogToc * toc) = 0;
	virtual void logBlockReceived(CFLogBlock * block) = 0;
	*/

	virtual void safeLinkReceived();
	void updateQualityPackets(bool val);


	void stateChanged();

	void stopAllTimers();
	void startTimer(TimerId id);
	virtual void timerCallback(int timerID) override;



	//
	WeakReference<CFDrone>::Master masterReference;
};