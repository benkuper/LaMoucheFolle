/*
  ==============================================================================

    Drone.h
    Created: 25 Nov 2019 2:38:27pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "Crazyflie.h"
#include "DroneDefs.h"
#include "Radio/CFParam.h"
#include "Radio/CFLog.h"
#include "Radio/CFCommand.h"
#include "Radio/CFPacket.h"

class Drone :
	public BaseItem,
	public Thread,
	public Timer
{
public:
	Drone();
	~Drone();

	enum State { DISCONNECTED, CONNECTING, READY, WARNING, TAKING_OFF, FLYING, LANDING, STATE_MAX };
	const String stateNames[STATE_MAX] { "Disconnected", "Connecting", "Ready", "Warning", "Taking off", "Flying", "Landing" };
	
	enum LightMode { OFF, WHITE_SPINNER, COLOR_SPINNER, TILT_EFFECT, BRIGHTNESS, COLOR_SPINNER2, DOUBLE_SPINNER, SOLID_COLOR, FACTORY_TEST, BATTERY_STATUS, BOAT_LIGHTS, ALERT, GRAVITY, MEMORY, FADE_COLOR, LIGHTMODE_MAX };
	const String lightModeNames[LIGHTMODE_MAX]{ "Off","White spinner","Color spinner","Tilt effect","Brightness","Color spinner2","Double spinner","Solid color","Factory test","Battery status","Boat lights","Alert","Gravity","Memory","Fade Color" };

	ControllableContainer infoCC;
	IntParameter* id;
	EnumParameter* state;
	FloatParameter* battery;
	FloatParameter* linkQuality;

	ControllableContainer controlCC;
	Trigger* takeOffTrigger;
	Trigger* landTrigger;
	Trigger* stopTrigger;
	Trigger* rebootTrigger;
	Trigger* offTrigger;
	Trigger* onTrigger;
	
	ControllableContainer flightCC;
	//FloatParameter* flightSmoothing;
	Point3DParameter* desiredPosition;
	Point3DParameter* desiredSpeed;
	Point3DParameter* desiredAcceleration;

	ControllableContainer lightsCC;
	EnumParameter* lightMode;
	ColorParameter* color;
	FloatParameter* fadeTime;
	BoolParameter* headlight;
	BoolParameter* stealthMode;

	Point3DParameter* realPosition;
	Point3DParameter* realRotation;
	BoolParameter * enableYawLookAt;
	Point2DParameter* yawLookAt;
	FloatParameter* targetYaw;
	

	String consoleBuffer;

	//Drone data
	CFParamToc* paramToc;
	CFLogToc* logToc;

	ControllableContainer decksCC;

	double lastPhysicsUpdateTime;

	//Lighthouse
	//sCrazyflie::MemoryTocEntry lighthouseMemoryEntry;

	void onContainerParameterChangedInternal(Parameter* c) override;
	void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) override;

	void ping();
	void takeOff();
	void land();
	void stop();
	void reboot();
	void setSystemPower(bool power);
	void setSystemLed(bool isOn);

	void setParam(StringRef name, var value);
	void setPosition(Vector3D<float> position, float yaw,  float time); // expected Y axis as UP

	bool canFly();
	bool isFlying();

	bool safeLinkActive;
	bool safeLinkUpFlag;
	bool safeLinkDownFlag;

	Array<CFCommand *, CriticalSection> uniqueCommands;
	Array<CFCommand *, CriticalSection> commands;
	
	void updateCustomDecks();

	void addCommand (CFCommand * command, bool force = false);
	void addUniqueCommand(CFCommand * command);

	void clearCommands();

	void connect();
	void setupDrone();
	void disconnect();

	void setLighthouseSetup();

	String getURI() const;

	void sendPosition();
	float getDesiredYaw();

	//Events
	void noAckReceived();

	void packetReceived(CFPacket * packet);

	void consolePacketReceived(const String &data);
	void rssiAckReceived(int data);

	void paramTocReceived(int crc, int size);
	void paramInfoReceived(int id, String group, String name, int type, bool readOnly, int length, int sign);

	void logTocReceived(int crc, int size);
	void logVariableInfoReceived(int id, String group, String name, int type);

	void logDataReceived(int blockId, var data);

	void run() override;

	void timerCallback() override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;

	String getTypeString() const override { return "Drone"; }

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

	WeakReference<Drone>::Master masterReference;
};