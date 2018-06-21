/*
  ==============================================================================

	CFDrone.cpp
	Created: 19 Jun 2018 8:38:43am
	Author:  Ben

  ==============================================================================
*/

#include "CFDrone.h"
#include "CFSettings.h"

CFDrone::CFDrone() :
	BaseItem("CFDrone"),
	paramToc(nullptr),
	safeLinkActive(false),
	safeLinkDownFlag(false),
	safeLinkUpFlag(false),
	qualityPacketIndex(0),

	radioCC("Radio"),
	controlsCC("Controls"),
	statusCC("Status"),
	decksCC("Decks"),
	wingsCC("Wings"),
	flightCC("Flight"),
	lightingCC("Lights"),
	lastPingTime(0),
	timeAtStartTakeOff(0),
	timeAtStartLanding(0),
	timeAtStartConverge(0),
	lastPhysicsUpdateTime(0)
{
	droneId = addIntParameter("ID", "Id of the drone, auto-defining channel and address.\nId of 0 will be adress 0xE7E7E7E700 and channel 0. Id of 1 will be 0xE7E7E7E701 and channel 2", 0, 0, 60);

	addChildControllableContainer(&statusCC);
	state = statusCC.addEnumParameter("Drone State", "State of the drone");
	state->addOption("Powered Off", POWERED_OFF)->addOption("Disconnected", DISCONNECTED)->addOption("Connecting", CONNECTING)
		->addOption("Calibrating", CALIBRATING)->addOption("Analysis", ANALYSIS)
		->addOption("Warning", WARNING)->addOption("Ready", READY)->addOption("Taking Off", TAKING_OFF)
		->addOption("Flying", FLYING)->addOption("LANDING", LANDING)->addOption("Error", ERROR);
	linkQuality = statusCC.addFloatParameter("Link Quality", "Quality of radio communication with the drone", 0, 0, 1);

	batteryLevel = statusCC.addFloatParameter("BatteryLevel", "Voltage of the drone. /!\\ When flying, there is a massive voltage drop !", 0, 0, 100);
	charging = statusCC.addBoolParameter("Charging", "Is the drone charging", false);
	lowBattery = statusCC.addBoolParameter("Low Battery", "Low battery", false);

	statusCC.addChildControllableContainer(&decksCC);
	decksCC.editorIsCollapsed = true;
	for (int i = 0; i < MAX_DECKS; i++)
	{
		EnumParameter * dc = decksCC.addEnumParameter("Deck #" + String(i), "Detected deck at index " + String(i) + " on the drone");
		for (int j = 0; j < DECKID_MAX; j++) dc->addOption(deckNames[j], (DeckId)j);
	}

	for (auto &c : decksCC.controllables) c->setControllableFeedbackOnly(true);

	statusCC.addChildControllableContainer(&wingsCC);
	wingsCC.editorIsCollapsed = true;
	for (int i = 0; i < 4; i++)
	{
		BoolParameter * p = wingsCC.addBoolParameter("Wing " + String(i), "If this is not checked, there is a problem either on this motor or wing", false);
		p->setControllableFeedbackOnly(true);
	}


	addChildControllableContainer(&controlsCC);
	connectTrigger = controlsCC.addTrigger("Connect", "Connect to the drone");
	calibrateTrigger = controlsCC.addTrigger("Calibrate", "Reset the kalman filter");
	takeOffTrigger = controlsCC.addTrigger("Take off", "Make the drone take off");
	rebootTrigger = controlsCC.addTrigger("Reboot", "Reboot the drone");
	stopTrigger = controlsCC.addTrigger("Stop", "Stop the drone");
	landTrigger = controlsCC.addTrigger("Land", "Land the drone");

	addChildControllableContainer(&flightCC);
	desiredPosition = flightCC.addPoint3DParameter("Desired Position", "The desired position for the drone. This is the one you setup");
	desiredSpeed = flightCC.addPoint3DParameter("Desired Speed", "The desired speed for the drone. This is the one you setup");
	desiredAcceleration = flightCC.addPoint3DParameter("Desired Acceleration", "The desired acceleration for the drone. This is the one you setup");

	targetPosition = flightCC.addPoint3DParameter("Target Position", "Computed target position for the drone depending on desired parameters and the physics engine. This is the one sent to the drone");
	targetSpeed = flightCC.addPoint3DParameter("Target Speed", "Computed target position for the drone depending on desired parameters and the physics engine. This is the one sent to the drone");
	targetAcceleration = flightCC.addPoint3DParameter("Target Acceleration", "Computed target position for the drone depending on desired parameters and the physics engine.This is the one sent to the drone");

	targetPosition->isControllableFeedbackOnly = true;
	targetSpeed->isControllableFeedbackOnly = true;
	targetAcceleration->isControllableFeedbackOnly = true;

	yaw = flightCC.addFloatParameter("Yaw", "The target horizontal rotation of the drone, 0 is aligned to X+", 0, 0, 360);
	realPosition = flightCC.addPoint3DParameter("Real Position", "Real Position feedback from the drone");

	addChildControllableContainer(&lightingCC);
	lightMode = lightingCC.addEnumParameter("LightMode", "Led Preset");
	for (int i = 0; i < LIGHTMODE_MAX; i++) lightMode->addOption(lightModeNames[i], (LightMode)i);

	color = lightingCC.addColorParameter("Light Color", "The color of the led ring. Only works in fade color mode", Colours::black);
	fadeTime = lightingCC.addFloatParameter("Fade time", "The time to fade from one color to another", .5f, 0, 10);
	color->isSavable = false;
	headlight = lightingCC.addBoolParameter("Headlight", "Headlight", false);
	headlight->isSavable = false;
	stealthMode = lightingCC.addBoolParameter("Stealth Mode", "When in stealthMode, the system leds are off", false);


	selfTestProblem = statusCC.addBoolParameter("SelfTest Problem", "There was a problem during the selfTest", false);
	batteryProblem = statusCC.addBoolParameter("Battery Problem", "If on, you should replace this battery !", false);
	for (auto &c : statusCC.controllables) c->setControllableFeedbackOnly(true);

	calibrationProgress = statusCC.addFloatParameter("Calibration Progress", "Progress of calibration", 0, 0, 1);
	analysisProgress = statusCC.addFloatParameter("Analysis Progress", "Progress of analysis", 0, 0, 1);

	memset(qualityPackets, 0, maxQualityPackets * sizeof(bool));

	//addCommand(CFCommand::createActivateSafeLink(this));

	stateChanged(); //force powered_off take care
}

CFDrone::~CFDrone()
{
	masterReference.clear();
}

void CFDrone::addCommand(CFCommand * command)
{
	if (!enabled->boolValue()) return;

	commandQueue.getLock().enter();
	commandQueue.add(command);
	commandQueue.getLock().exit();
}

CFCommand * CFDrone::getDefaultCommand()
{
	if (!enabled->boolValue()) return nullptr;

	DroneState s = state->getValueDataAsEnum<DroneState>();
	if (s == POWERED_OFF) return nullptr;
	return CFCommand::createPing(this);
}

void CFDrone::addConnectionCommands()
{
	//Get param tocs
	addCommand(CFCommand::createRequestParamToc(this));
	//Get log tocs
	addCommand(CFCommand::createRequestLogToc(this));

	//Register log
	//Create a status log, low freq
	//Create a position/orientation log, high freq

	//check self-test

	//connected, pass to flight analysis
	//state->setValueWithData(ANALYSIS);
	//or pass to calibration until analysis is implemented
	//state->setValueWithData(READY);
}

void CFDrone::addTakeoffCommand()
{
	float t = (Time::getMillisecondCounter() - timeAtStartTakeOff) / 1000.0f;
	float relTime = t / CFSettings::getInstance()->takeOffTime->floatValue();

	if (relTime >= 1)
	{
		state->setValueWithData(FLYING);
		return;
	}

	float velZ = CFSettings::getInstance()->takeOffCurve.getValueForPosition(relTime) * CFSettings::getInstance()->takeOffMaxSpeed->floatValue();
	DBG("Vel z " << velZ);
	addCommand(CFCommand::createVelocity(this, Vector3D<float>(0, 0, velZ), 0));
}

void CFDrone::addFlyingCommand()
{
	float deltaTime = (Time::getMillisecondCounter() - lastPhysicsUpdateTime) / 1000.0f;

	PhysicsCC::PhysicalState currentState = PhysicsCC::PhysicalState(targetPosition->getVector(), targetSpeed->getVector(), targetAcceleration->getVector());
	PhysicsCC::PhysicalState desiredState = PhysicsCC::PhysicalState(desiredPosition->getVector(), desiredSpeed->getVector(), desiredAcceleration->getVector());
	PhysicsCC::PhysicalState targetState = CFSettings::getInstance()->physicsCC.processPhysics(deltaTime, currentState, desiredState);
	targetPosition->setVector(targetState.position);
	targetSpeed->setVector(targetState.speed);
	targetAcceleration->setVector(targetState.acceleration);
	addCommand(CFCommand::createPosition(this, targetState.position, yaw->floatValue()));
}

void CFDrone::addLandingCommand()
{
	addCommand(CFCommand::createVelocity(this, Vector3D<float>(0, -.5f, 0), 0));
	if (Time::getMillisecondCounter() > timeAtStartLanding + landingTime) state->setValueWithData(READY);
}

void CFDrone::addParamCommand(String paramName, var value)
{
	addCommand(CFCommand::createSetParam(this, paramName, value));
}

void CFDrone::addRebootCommand()
{
	addCommand(CFCommand::createRebootInit(this));
	addCommand(CFCommand::createRebootFirmware(this));
}

void CFDrone::syncToRealPosition()
{
	targetPosition->setVector(realPosition->getVector());
	desiredPosition->setVector(realPosition->getVector());
}

void CFDrone::updateControls()
{
	DroneState s = state->getValueDataAsEnum<DroneState>();
	connectTrigger->setEnabled(s == DISCONNECTED);
	calibrateTrigger->setEnabled(s == READY || s == WARNING);
	takeOffTrigger->setEnabled(s == READY);
	landTrigger->setEnabled(s == FLYING);
	rebootTrigger->setEnabled(s != TAKING_OFF && s != FLYING && s != LANDING);
	stopTrigger->setEnabled(s == READY || s == TAKING_OFF || s == FLYING || s == LANDING);
}


void CFDrone::onContainerParameterChangedInternal(Parameter * p)
{
	if (p == enabled)
	{
		DroneState s = state->getValueDataAsEnum<DroneState>();
		state->setValueWithData(POWERED_OFF);
		if (s == POWERED_OFF || !enabled->boolValue()) stateChanged(); //force when not enabled or was already in this state to stop timers
	}
}

void CFDrone::onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c)
{
	DroneState currentState = state->getValueDataAsEnum<DroneState>();
	if (c == state && currentState == POWERED_OFF) updateControls();

	if (!enabled->boolValue()) return;

	if (c == state) stateChanged();
	else if (c == connectTrigger) state->setValueWithData(CONNECTING);
	else if (c == calibrateTrigger) state->setValueWithData(CONNECTING);
	else if (c == rebootTrigger)
	{
		addRebootCommand();
		state->setValueWithData(POWERED_OFF);
	}else if (c == stopTrigger)
	{
		addCommand(CFCommand::createStop(this));
		if(currentState == FLYING ||currentState == TAKING_OFF || currentState == LANDING) state->setValueWithData(READY);
	} else if (c == takeOffTrigger)
	{
		if (currentState == READY) state->setValueWithData(TAKING_OFF);
	} else if (c == landTrigger)
	{
		if (currentState == FLYING || currentState == TAKING_OFF) state->setValueWithData(LANDING);
	}
}

void CFDrone::noAckReceived()
{
	if (state->getValueDataAsEnum<DroneState>() != POWERED_OFF)
	{
		//DBG("No ack received");
	}

	updateQualityPackets(false);
	if (linkQuality->floatValue() == 0)
	{
		//DBG("TIMEOUT, Set POWERED_OFF");
		state->setValueWithData(POWERED_OFF);
	}
}

void CFDrone::packetReceived(const CFPacket & packet)
{
	//DBG("Packet received " << packet.getTypeString());
	updateQualityPackets(true);
	if (state->getValueDataAsEnum<DroneState>() == POWERED_OFF)
	{
		//DBG("Set disconnected here");
		state->setValueWithData(DISCONNECTED);
	}
	switch (packet.type)
	{
	case CFPacket::CONSOLE: consolePacketReceived(packet.data.toString()); break;
	case CFPacket::RSSI_ACK: rssiAckReceived((int)packet.data); break;
	case CFPacket::SAFELINK: safeLinkReceived(); break;
	case CFPacket::PARAM_TOC_INFO: paramTocReceived((int)packet.data.getProperty("crc", 0), (int)packet.data.getProperty("size", 0)); break;
	case CFPacket::PARAM_TOC_ITEM: paramInfoReceived(
		(int)packet.data.getProperty("id", -1),
		packet.data.getProperty("group","[notset]").toString(),
		packet.data.getProperty("name", "[notset]").toString(),
		(int)packet.data.getProperty("type", -1),
		(bool)packet.data.getProperty("readOnly", 0),
		(int)packet.data.getProperty("length", -1),
		(int)packet.data.getProperty("sign", -1));
	break;
	}
}

void CFDrone::consolePacketReceived(const String &msg)
{
	if (msg.containsChar('\n'))
	{
		consoleBuffer << msg.substring(0, msg.indexOfChar('\n'));

		String lowerCase = consoleBuffer.toLowerCase();
		if (lowerCase.contains("error") || lowerCase.contains("fail")) NLOGERROR(niceName, consoleBuffer);
		else if (lowerCase.contains("warning")) NLOGWARNING(niceName, consoleBuffer);
		else NLOG(niceName, consoleBuffer);
		consoleBuffer.clear();
	} else
	{
		String c = msg.substring(0, msg.indexOfChar((char)1));
		consoleBuffer << c;

	}
}

void CFDrone::rssiAckReceived(uint8_t)
{
	//DBG("RSSI Ack : " << rssi);
}

void CFDrone::paramTocReceived(int crc, int size)
{
	if (crc == 0 || size == 0)
	{
		NLOGWARNING(niceName,"Wrong ParamTOC Info received : " << crc << ", " << size);
		return;
	}

	paramToc = CFParamToc::addParamToc(crc, size);
	DBG("Param toc received : CRC " << paramToc->crc << ", " << paramToc->numParams << " parameters");
	
	if (!paramToc->isInitialized())
	{
		for (int i = 0; i < paramToc->numParams; i++) addCommand(CFCommand::createGetParamInfo(this, i));
	}
}

void CFDrone::paramInfoReceived(int paramId, String group,String name, int type, bool readOnly, int length, int sign)
{
	LOG("Param info received : " << group <<  ", " << name << ", " << type << ", " << (int)readOnly << ", " << length << ", " << sign);
}

void CFDrone::paramValueReceived(CFParam * param)
{
}

void CFDrone::safeLinkReceived()
{
	NLOG(niceName, "Safe link activated");
	safeLinkActive = true;
}

void CFDrone::updateQualityPackets(bool val)
{
	bool oldVal = qualityPackets[qualityPacketIndex];
	if (oldVal != val)
	{
		qualityPackets[qualityPacketIndex] = val;

		float p = 0;
		for (int i = 0; i < maxQualityPackets; i++) p += (int)qualityPackets[i];
		linkQuality->setValue(p/maxQualityPackets);

	}
	qualityPacketIndex = (qualityPacketIndex + 1) % maxQualityPackets;

}

void CFDrone::stateChanged()
{
	DroneState s = state->getValueDataAsEnum<DroneState>();

	NLOG(niceName, "State update : " << state->getValueKey());
	
	updateControls();

	stopAllTimers();

	if (!enabled->boolValue()) return;

	switch (s)
	{
	case POWERED_OFF:
		startTimer(TIMER_PING);
		break;

	case DISCONNECTED:
	{
		//addCommand(CFCommand::createActivateSafeLink(this));
		if (CFSettings::getInstanceWithoutCreating() != nullptr && CFSettings::getInstance()->autoConnect->boolValue()) state->setValueWithData(CONNECTING);
	}
	break;

	case CONNECTING:
		addConnectionCommands();
		break;

	case CALIBRATING:
	{
		addParamCommand("kalman.resetEstimation", 1);
		addParamCommand("kalman.resetEstimation", 0);
		timeAtStartCalib = Time::getMillisecondCounter();
		timeAtStartConverge = 0;
		lightMode->setValueWithData(DOUBLE_SPINNER);
		startTimer(TIMER_CALIBRATION);
	}
	break;

	case ANALYSIS:
	{
		lightMode->setValueWithData(BATTERY_STATUS);
	}
	break;

	case READY:
	{
		lightMode->setValueWithData(FADE_COLOR);
		fadeTime->setValue(.5f);
		color->setColor(Colours::black);
		syncToRealPosition();
	}
	break;

	case TAKING_OFF:
	{
		timeAtStartTakeOff = Time::getMillisecondCounter();
		startTimer(TIMER_TAKEOFF);
	}
	break;

	case FLYING:
	{
		startTimer(TIMER_FLYING);
	}
	break;

	case LANDING:
	{
		timeAtStartLanding = Time::getMillisecondCounter();
		float h = CFSettings::getInstance()->zIsVertical->boolValue() ? realPosition->z : realPosition->y;
		landingTime = jmax<float>(h * 2, 1);

		startTimer(TIMER_LANDING);
	}
	break;

	case WARNING:
		lightMode->setValueWithData(BOAT_LIGHTS);
		break;

	case ERROR:
		lightMode->setValueWithData(ALERT);
		break;

	}

}

void CFDrone::stopAllTimers()
{
	for (int i = 0; i < TIMER_MAX; i++) stopTimer(i);
}

void CFDrone::startTimer(TimerId id)
{
	MultiTimer::startTimer(id, (int)(1000.0f / timerFreqs[id]));
}

void CFDrone::timerCallback(int timerID)
{
	switch ((TimerId)timerID)
	{

	case TIMER_PING:
	{
		addCommand(CFCommand::createPing(this));
	}
	break;

	case TIMER_TAKEOFF:
	{
		addTakeoffCommand();
	}
	break;

	case TIMER_FLYING:
	{
		addFlyingCommand();
	}
	break;

	case TIMER_LANDING:
	{
		addLandingCommand();
	}
	break;

	case TIMER_CALIBRATION:
	{
		float t = Time::getMillisecondCounter();
		if (timeAtStartConverge > 0 && t > timeAtStartConverge + minConvergeTime)
		{
			state->setValueWithData(READY);
		}

		if (t > timeAtStartCalib + calibTimeout)
		{
			NLOGWARNING(niceName, "Calib took too much time to converge !");
			state->setValueWithData(WARNING);
		}
	}
	break;

	default:
		//not handled
		break;
	}
}
