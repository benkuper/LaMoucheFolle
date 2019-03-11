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
	logToc(nullptr),
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
	isCalibrated(false),
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

	batteryLevel = statusCC.addFloatParameter("Battery Level", "Computed battery level of the drone. /!\\ When flying, there is a massive voltage drop !", 0, 0, 1);
	voltage = statusCC.addFloatParameter("Voltage", "Voltage of the drone. /!\\ When flying, there is a massive voltage drop !", 4.2f, 2.9f, 4.2f);
	charging = statusCC.addBoolParameter("Charging", "Is the drone charging", false);
	lowBattery = statusCC.addBoolParameter("Low Battery", "Low battery", false);

	statusCC.addChildControllableContainer(&decksCC);
	decksCC.editorIsCollapsed = true;
	for (int i = 0; i < DECKID_MAX; i++)
	{
		BoolParameter * dc = decksCC.addBoolParameter("Deck " + deckNames[i], "Is the deck " + deckIds[i] + " connected ?", false);
		deckMap.set(deckIds[i], dc);
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
	tocTrigger = controlsCC.addTrigger("Request TOC", "Request TOCs");
	calibrateTrigger = controlsCC.addTrigger("Calibrate", "Reset the kalman filter");
	takeOffTrigger = controlsCC.addTrigger("Take off", "Make the drone take off");
	takeOffHeight = controlsCC.addFloatParameter("TakeOff Height", "Height to take off the drone", 1.5f, .5f, 5);
	rebootTrigger = controlsCC.addTrigger("Reboot", "Reboot the drone");
	stopTrigger = controlsCC.addTrigger("Stop", "Stop the drone");
	landTrigger = controlsCC.addTrigger("Land", "Land the drone");
	setupNodesTrigger = controlsCC.addTrigger("Setup Nodes", "Send node positions");
	propCheckTrigger = controlsCC.addTrigger("Propeller Check", "Check propellers. May be good to restart after");

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

	realPosition = flightCC.addPoint3DParameter("Real Position", "Real Position feedback from the drone");
	positionDiff = flightCC.addPoint3DParameter("Position Diff", "Diff position");
	positionDiff->setBounds(-5, -5, -5, 5, 5, 5);
	positionNoise = flightCC.addFloatParameter("Position Noise", "Overall position noise",0,0,5);

	targetYaw = flightCC.addFloatParameter("Target Yaw", "Target horizontal orientation", 0, -180, 180);
	orientation = flightCC.addPoint3DParameter("Orientation", "The target rotation of the drone, Y = 0 is aligned to X+");
	orientation->setBounds(-90, -180, -180, 90, 180, 180);
	upsideDown = flightCC.addBoolParameter("Upside Down", "Is the drone upside down ?", false);

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

//This function is called multiple times and react differently depending on the current state
void CFDrone::addConnectionCommands()
{

	//Get param tocs
	if (paramToc == nullptr || !paramToc->isInitialized())
	{
		LOG("Request Param TOC");
		addCommand(CFCommand::createRequestParamToc(this));
		return;
	}

	//Get log tocs
	if (logToc == nullptr || !logToc->isInitialized())
	{
		LOG("Request Log TOC");
		addCommand(CFCommand::createRequestLogToc(this));
		return;
	}

	//Sync current parameters
	for (auto &p : paramToc->params)
	{
		addGetParamValueCommand(p->definition.id);
	}

	//Reset all logs
	addCommand(CFCommand::createResetLogs(this));

	//Create a status log, low freq
	addCommand(CFCommand::createAddLogBlock(this, LOG_POWER_ID, Array<String>("pm.batteryLevel", "pm.vbat", "pm.state")));

	//Create a position/orientation log, high freq
	addCommand(CFCommand::createAddLogBlock(this, LOG_POSITION_ID, Array<String>("stateEstimate.x", "stateEstimate.y", "stateEstimate.z", "stabilizer.pitch", "stabilizer.yaw", "stabilizer.roll")));

	//Create calib command but do not start it here
	addCommand(CFCommand::createAddLogBlock(this, LOG_CALIB_ID, Array<String>("kalman.varPX", "kalman.varPY", "kalman.varPZ")));


	addCommand(CFCommand::createStartLog(this, LOG_POWER_ID, 5));
	addCommand(CFCommand::createStartLog(this, LOG_POSITION_ID, 20));


	//check self-test

	//connected, pass to flight analysis
	//state->setValueWithData(ANALYSIS);
	//or pass to calibration until analysis is implemented

	int lpsMode = (int)CFSettings::getInstance()->lpsMode->getValueData();
	if (lpsMode > -1)
	{
		NLOG(niceName, "Set LPS Mode to " << (lpsMode == 0 ? "Auto" : "TDoA " + String(lpsMode)));
		addParamCommand("loco.mode", lpsMode);
	} else
	{
		NLOG(niceName, "Not setting LPS Mode");
	}

	if (CFSettings::getInstance()->calibAfterConnect->boolValue())
	{
		NLOG(niceName, "Initialized, now calibrating");
		state->setValueWithData(CALIBRATING);
	}
	else state->setValueWithData(READY);
}

void CFDrone::addTakeoffCommand()
{
	float t = (Time::getMillisecondCounter() - timeAtStartTakeOff) / 1000.0f;
	float relTime = t / CFSettings::getInstance()->takeOffTime->floatValue();

	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	float realHeight = zIsVertical ? realPosition->z : realPosition->y;
	bool heightIsGood = Time::getMillisecondCounter() > timeAtStartLanding + 1 && realHeight >= takeOffHeight->floatValue();

	if (relTime >= 1 || heightIsGood)
	{
		state->setValueWithData(FLYING);
		return;
	}

	if (CFSettings::getInstance()->useThrustCommand->boolValue())
	{
		float minTakeOff = CFSettings::getInstance()->takeOffMinSpeed->floatValue();
		float maxTakeOff = CFSettings::getInstance()->takeOffMaxSpeed->floatValue();
		float thrust = minTakeOff + CFSettings::getInstance()->takeOffCurve.getValueForPosition(relTime) * (maxTakeOff - minTakeOff);

		addCommand(CFCommand::createSetPoint(this, 0, 0, 0, thrust * 10000));
		LOG("add thrust takeoff command with thrust " << thrust);
	} else
	{
		float velZ = CFSettings::getInstance()->takeOffCurve.getValueForPosition(relTime) * CFSettings::getInstance()->takeOffMaxSpeed->floatValue();
		LOG("add velocity takeoff command with force " << velZ);
		addCommand(CFCommand::createVelocity(this, Vector3D<float>(0, 0, velZ), 0));
	}

}

void CFDrone::addFlyingCommand()
{
	double deltaTime = Time::getMillisecondCounter() / 1000.0 - lastPhysicsUpdateTime;

	PhysicsCC::PhysicalState currentState = PhysicsCC::PhysicalState(targetPosition->getVector(), targetSpeed->getVector(), targetAcceleration->getVector());
	PhysicsCC::PhysicalState desiredState = PhysicsCC::PhysicalState(desiredPosition->getVector(), desiredSpeed->getVector(), desiredAcceleration->getVector());
	PhysicsCC::PhysicalState targetState = CFSettings::getInstance()->physicsCC.processPhysics(deltaTime, currentState, desiredState);
	targetPosition->setVector(targetState.position);
	targetSpeed->setVector(targetState.speed);
	targetAcceleration->setVector(targetState.acceleration);

	//Invert Z and Y depending on zIsVertical option in Project Settings (if zIsVertical is not checked, then don't swap y and z because crazyflie consider z as vertical)
	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	Vector3D<float> targetPos(targetState.position.x, zIsVertical ? targetState.position.y : targetState.position.z, zIsVertical ? targetState.position.z : targetState.position.y);
	addCommand(CFCommand::createPosition(this, targetPos, CFSettings::getInstance()->disableYawCommand->boolValue()?0:targetYaw->floatValue()));
	lastPhysicsUpdateTime = Time::getMillisecondCounter() / 1000.0;
}

void CFDrone::addLandingCommand()
{
	//Reset the yaw and turn the drone
	targetYaw->setValue(0);
	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	Vector3D<float> p = targetPosition->getVector();
	Vector3D<float> targetPos(p.x, zIsVertical ? p.y : p.z, zIsVertical ? p.z : p.y);
	addCommand(CFCommand::createPosition(this, targetPos, 0));

	addCommand(CFCommand::createVelocity(this, Vector3D<float>(0, 0, -.75f), 0));
	if(Time::getMillisecondCounter() > timeAtStartLanding + landingTime * 1000) state->setValueWithData(READY);
}

void CFDrone::addParamCommand(String paramName, var value)
{
	addCommand(CFCommand::createSetParam(this, paramName, value));
}

void CFDrone::addGetParamValueCommand(int paramId)
{
	addCommand(CFCommand::createGetParamValue(this, paramId));
}

void CFDrone::addGetParamValueCommand(String paramName)
{
	if (paramToc == nullptr)
	{
		DBG("Param toc is null, doing nothing");
		return;
	}
	int pid = paramToc->getParamIdForName(paramName);
	addGetParamValueCommand(pid);
}

void CFDrone::addRebootCommand()
{
	safeLinkActive = false;
	addCommand(CFCommand::createRebootInit(this));
	addCommand(CFCommand::createRebootFirmware(this));
}

void CFDrone::addSetupNodesCommands()
{
	//addCommand(CFCommand::createGetMemoryNumber(this));
	
	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	Vector3D<float> pos = CFSettings::getInstance()->lpsBoxSize->getVector();
	if (!zIsVertical) pos = Vector3D<float>(pos.x, pos.z, pos.y);

	float zOffset = CFSettings::getInstance()->lpsZOffset->floatValue();
	pos.z += zOffset;
	
	/*
	for (int i = 0; i < 8; i++)
	{
		addGetParamValueCommand("anchorpos.anchor" + String(i) + "x");
		addGetParamValueCommand("anchorpos.anchor" + String(i) + "y");
		addGetParamValueCommand("anchorpos.anchor" + String(i) + "z");
	}
	*/

	addCommand(CFCommand::createLPSNodePos(this, 0, -pos.x / 2, -pos.y / 2, zOffset));
	addCommand(CFCommand::createLPSNodePos(this, 1, -pos.x / 2, -pos.y / 2, pos.z));

	addCommand(CFCommand::createLPSNodePos(this, 2, -pos.x / 2, pos.y / 2, zOffset));
	addCommand(CFCommand::createLPSNodePos(this, 3, -pos.x / 2, pos.y / 2, pos.z));

	addCommand(CFCommand::createLPSNodePos(this, 4, pos.x / 2, pos.y / 2, zOffset));
	addCommand(CFCommand::createLPSNodePos(this, 5, pos.x / 2, pos.y / 2, pos.z));

	addCommand(CFCommand::createLPSNodePos(this, 6, pos.x / 2, -pos.y / 2, zOffset));
	addCommand(CFCommand::createLPSNodePos(this, 7, pos.x / 2, -pos.y / 2, pos.z));
	
}


void CFDrone::syncToRealPosition()
{
	targetPosition->setVector(realPosition->getVector());
	desiredPosition->setVector(realPosition->getVector());
	lastPhysicsUpdateTime = Time::getMillisecondCounter() / 1000.0;
}

void CFDrone::updateControls()
{
	DroneState s = state->getValueDataAsEnum<DroneState>();
	connectTrigger->setEnabled(s == DISCONNECTED);
	tocTrigger->setEnabled(s != POWERED_OFF && s != DISCONNECTED);
	calibrateTrigger->setEnabled(s == READY || s == WARNING);
	takeOffTrigger->setEnabled(s == READY);
	landTrigger->setEnabled(s == TAKING_OFF || s == FLYING);
	rebootTrigger->setEnabled(s != TAKING_OFF && s != FLYING && s != LANDING);
	stopTrigger->setEnabled(s == READY || s == TAKING_OFF || s == FLYING || s == LANDING);
	setupNodesTrigger->setEnabled(s != POWERED_OFF);
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
	else if (c == tocTrigger) addConnectionCommands();
	else if (c == calibrateTrigger) state->setValueWithData(CALIBRATING);
	else if (c == rebootTrigger)
	{
		state->setValueWithData(POWERED_OFF);
		addRebootCommand();
	} else if (c == stopTrigger)
	{
		addCommand(CFCommand::createStop(this));
		if (currentState == FLYING || currentState == TAKING_OFF || currentState == LANDING) state->setValueWithData(READY);
	} else if (c == takeOffTrigger)
	{
		if (currentState == READY && !lowBattery->boolValue() && !upsideDown->boolValue()) state->setValueWithData(TAKING_OFF);
	} else if (c == landTrigger)
	{
		if (currentState == FLYING || currentState == TAKING_OFF) state->setValueWithData(LANDING);
	} else if (c == setupNodesTrigger)
	{
		if (currentState != POWERED_OFF) addSetupNodesCommands();
	} else if (c == propCheckTrigger)
	{
		addParamCommand("health.startPropTest", 1);
		addParamCommand("health.startPropTest", 0);
	}

	//Position testing
	else if(c == realPosition)
	{
		double time = Time::getMillisecondCounterHiRes() / 1000.0;
		double deltaTime = time - lastRealPosTime;

		positionDiff->setVector((realPosition->getVector() - lastRealPos) / deltaTime);
		positionNoise->setValue(positionDiff->getVector().length());

		lastRealPosTime = time;
		lastRealPos = realPosition->getVector();
	}

	//Battery
	else if (c == lowBattery)
	{
		if (lowBattery->boolValue())
		{
			MultiTimer::startTimer(LOWBAT_BLINK_ID, 2000);
			landTrigger->trigger();
		} else
		{
			stopTimer(LOWBAT_BLINK_ID);
		}
	} else if (c == charging)
	{
		lowBattery->setValue(false);
		if (charging->boolValue()) color->setColor(Colours::yellow.darker());
		else color->setColor(Colours::black);
	} else if (c == upsideDown)
	{
		if (upsideDown->boolValue()) MultiTimer::startTimer(UPSIDE_DOWN_ID, 2000);
		else if (MultiTimer::isTimerRunning(UPSIDE_DOWN_ID)) stopTimer(UPSIDE_DOWN_ID);
	}

	//controls authorized when battery ok

	else if (!lowBattery->boolValue())
	{
		if (c == color) addParamCommand("ring.fadeColor", (int64)color->getColor().getARGB());
		else if (c == headlight) addParamCommand("ring.headlightEnable", headlight->boolValue());
		else if (c == lightMode) addParamCommand("ring.effect", lightMode->getValueData());
		else if (c == fadeTime) addParamCommand("ring.fadeTime", fadeTime->floatValue());
		else if (c == stealthMode) addParamCommand("platform.stealthMode", stealthMode->boolValue());
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

	case CFPacket::PARAM_TOC_INFO: paramTocReceived((int)packet.data.getProperty("crc", 0), (int)packet.data.getProperty("size", 0)); break;
	case CFPacket::PARAM_TOC_ITEM: paramInfoReceived(
		packet.data.getProperty("id", 0),
		packet.data.getProperty("group", "[notset]").toString(),
		packet.data.getProperty("name", "[notset]").toString(),
		(int)packet.data.getProperty("type", -1),
		(bool)packet.data.getProperty("readOnly", 0),
		(int)packet.data.getProperty("length", -1),
		(int)packet.data.getProperty("sign", -1));
		break;

	case CFPacket::LOG_TOC_INFO: logTocReceived((int)packet.data.getProperty("crc", 0), (int)packet.data.getProperty("size", 0)); break;
	case CFPacket::LOG_TOC_ITEM: logVariableInfoReceived(
		packet.data.getProperty("group", "[notset]").toString(),
		packet.data.getProperty("name", "[notset]").toString(),
		(int)packet.data.getProperty("type", -1));
		break;

	case CFPacket::LOG_CONTROL:
		//LOG("Log control : " << packet.data.getProperty("result", "[notset]").toString() << ", " << packet.data.getProperty("command", "[notset]").toString());
		break;

	case CFPacket::LOG_DATA:
		logBlockReceived(packet.data.getProperty("blockId", -1), packet.data.getProperty("data", var()));
		break;

	case CFPacket::PARAM_VALUE:
	{
		paramValueReceived(packet.data.getProperty("id", -1), packet.data.getProperty("value", var()));
	}
	break;

	case CFPacket::LPP_SHORT_PACKET:
	{
		//LOG("LPP Short Packet : " << packet.data.getProperty("x", "?").toString() << ", " << packet.data.getProperty("y", "").toString() << ", " << packet.data.getProperty("z", "").toString());
		//String s = "LPP Short Packet : ";
		//for(int i=0;i<packet.data.size();i++) s += packet.data[i].toString() +", "; 
		//LOG(s);
	}
	break;

	case CFPacket::MEMORY_NUMBER:
	{
		int number = packet.data;
		LOG("Got memory number " << number);
		for (int i = 0; i < number; i++)
		{
			addCommand(CFCommand::createGetMemoryInfo(this, i));
		}
	}
	break;

	case CFPacket::MEMORY_INFO:
	{
		int addr = packet.data.getProperty("address", -1);
		int size = packet.data.getProperty("size", -1);
		crtpMemoryType type = (crtpMemoryType)(int)packet.data.getProperty("type", -1);
		LOG("Got memory info " << addr << ", " << size << ", " << String::toHexString(type));

		if (type == crtpMemoryType::LPS_ANCHOR)
		{
			DBG("LPS Anchor memory, try to read anchor list");
			addCommand(CFCommand::createReadMemory(this, 0x13, 0x0, 1));
			addCommand(CFCommand::createReadMemory(this, 0x13, 0x1000, 1));
			addCommand(CFCommand::createReadMemory(this, 0x13, 0x2000, 13));
			addCommand(CFCommand::createReadMemory(this, 0x13, 0x3000, 13));
		}
	}
	break;

	case CFPacket::MEMORY_READ:
	{
		int addr = packet.data.getProperty("address", -1);
		int memoryId = packet.data.getProperty("memoryId", -1);
		int status = packet.data.getProperty("status", -1);
		LOG("Got memory read response 0x" << String::toHexString(addr) << ", 0x" << String::toHexString(memoryId) << ", " << status);
		var data = packet.data.getProperty("data", var());
		switch ((MemoryAddress)addr)
		{
		case ANCHOR_LIST:
		{
			LOG("Anchor list, " << data[0].toString() << " anchors");
			String s = "ids :";
			for (int i = 0; i < (int)data[0] && i < data.size()-1; i++)
			{
				s += data[i + 1].toString() + ", ";
			}
			LOG(s);
		}
		break;

		case ACTIVE_ANCHOR_LIST:
		{
			LOG("Active nchor list, " << data[0].toString() << " anchors");
			String s = "ids :";
			for (int i = 0; i < (int)data[0] && i < data.size()-1; i++)
			{
				s += data[i + 1].toString() + ", ";
			}
			LOG(s);
		}
		break;
		}

		if (addr >= ANCHOR_DATA)
		{
			int anchorId = floor((addr - 0x2000) / 0x1000);
			Array<uint8> bytes;
			for (int i = 0; i < 13; i++) bytes.add((int)data[i]);
			MemoryBlock b(bytes.getRawDataPointer(), bytes.size());
			MemoryInputStream stream(b, false);
			float x = stream.readFloat();
			float y = stream.readFloat();
			float z = stream.readFloat();
			uint8 saved = stream.readByte();
			LOG("Anchor data for anchor id " << anchorId << " : " << x << ", " << y << ", " << z << ", saved ? " << saved);
			break;
		}

	}
	break;

	case CFPacket::MEMORY_WRITE:
	{
		int addr = packet.data.getProperty("address", -1);
		int memoryId = packet.data.getProperty("memoryId", -1);
		int status = packet.data.getProperty("status", -1);
		LOG("Got memory write response " << addr << ", " << String::toHexString(memoryId) << ", " << status);

	}
	break;

	default:
		//not handled yet
		DBG("Unhandled packet : " << packet.getTypeString());
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
		NLOGWARNING(niceName, "Wrong ParamTOC Info received : " << crc << ", " << size);
		return;
	}

	paramToc = CFParamToc::addParamToc(crc, size);

	if (!paramToc->isInitialized())
	{
		DBG("Unknown toc received : CRC " << paramToc->crc << ", " << paramToc->numParams << " parameters");
		currentParamRequestId = 0; //reset Parameter id before asking all the parameters
		addCommand(CFCommand::createGetParamInfo(this, currentParamRequestId));
	} else
	{
		LOG("Found existing Param TOC");
		addConnectionCommands();
	}
}

void CFDrone::paramInfoReceived(int id, String group, String name, int type, bool readOnly, int length, int sign)
{
	if (currentParamRequestId == -1)
	{
		DBG("Received unexpected param info packet, probably from previous life");
		return;
	}

	DBG("Param received [" << id << "] : " << group << "." << name << " (" << CFParam::getTypeString((CFParam::Type)type) << ") " << (readOnly ? "readOnly" : "") << ", length : " << length << ", sign :" << sign);

	if (paramToc == nullptr)
	{
		//Drone has previous commands to send, ignoring them
		return;
	}

	if (paramToc->params.size() <= id)
	{
		paramToc->addParamDef(group + "." + name, id, (CFParam::Type)type);
		currentParamRequestId = id + 1;

		if (paramToc->isInitialized())
		{
			paramToc->save();
			addConnectionCommands(); // go to the next connection step
		} else
		{
			for (int i = 0; i < 2; i++) addCommand(CFCommand::createGetParamInfo(this, currentParamRequestId));
		}
	} else
	{
		//skip as it already exists
	}
}

void CFDrone::paramValueReceived(int id, var value)
{
	if (paramToc == nullptr)
	{
		DBG("Param toc null, can't process param value message");
		return;
	}

	CFParam * p = paramToc->getParam(id);
	if (p == nullptr)
	{
		LOGWARNING("Parameter not found with id " << id);
		return;
	}

	p->value = value;

	//Process param
	//LOG("Param value received : " << p->definition.name << " > " << value.toString());
	if (p->definition.group == "deck")
	{
		if (!deckMap.contains(p->definition.localName))
		{
			LOGWARNING(p->definition.localName << " not found in deck list");
			return;
		}
		deckMap[p->definition.localName]->setValue(value);
	} else if (p->definition.group == "anchorpos" && p->definition.localName != "enable")
	{
		int anchorId = p->definition.localName.substring(6, 7).getIntValue();
		char coord = p->definition.localName.getLastCharacter();

		Vector3D<float> box = CFSettings::getInstance()->lpsBoxSize->getVector();
		if (!CFSettings::getInstance()->zIsVertical->boolValue()) box = Vector3D<float>(box.x, box.z, box.y);

		float zOffset = CFSettings::getInstance()->lpsZOffset->floatValue();
		box.z += zOffset;

		float checkVal = 0;

		switch (coord)
		{
		case 'x':
		{
			checkVal = box.x / 2;
			if (anchorId == 0 || anchorId == 1 || anchorId == 2 || anchorId == 3) checkVal = -checkVal;
		}
		break;

		case 'y':
		{
			checkVal = box.y / 2;
			if (anchorId == 0 || anchorId == 1 || anchorId == 6 || anchorId == 7) checkVal = -checkVal;
		}
		break;

		case 'z':
		{
			checkVal = anchorId == 1 || anchorId == 3 || anchorId == 5 || anchorId == 7 ? box.z : zOffset;
		}
		break;
		}

		bool inSync = fabsf(checkVal - (float)p->value) < .05f;
		if (!inSync) LOGWARNING("Not in sync : " << anchorId << " (" << coord << ")");
		//else LOG("Anchor " << anchorId << " (" << coord << ") " << "ok");

	}
}

void CFDrone::logTocReceived(int crc, int size)
{
	if (crc == 0 || size == 0)
	{
		NLOGWARNING(niceName, "Wrong LogTOC Info received : " << crc << ", " << size);
		return;
	}

	//HACK because log variable returns 252
	size = 246;// size - 1;

	logToc = CFLogToc::addLogToc(crc, size);
	LOG("Log toc received : CRC " << logToc->crc << ", " << logToc->numVariables << " variables, is init ? " << (int)logToc->isInitialized());

	if (!logToc->isInitialized())
	{
		currentLogVariableId = logToc->variables.size(); //reset Parameter id before asking all the parameters

		//HACK skip 165
		if (currentLogVariableId == 164) currentLogVariableId++;

		DBG("Add request log item id " << currentLogVariableId);
		addCommand(CFCommand::createGetLogItemInfo(this, currentLogVariableId));

		String m = "missing : ";
		for (int i = 0; i < logToc->numVariables; i++)
		{
			if (logToc->getLogVariable(i) == nullptr) m += String(i) + ", ";
		}
		LOG("Missing variables are " << m);
	} else
	{
		LOG("Found existing log TOC");
		addConnectionCommands();
	}


}

void CFDrone::logVariableInfoReceived(String group, String name, int type)
{
	if (currentLogVariableId == -1)
	{
		DBG("Received unexpected logVariable info packet, probably from previous life");
		return;
	}

	if (logToc == nullptr)
	{
		//Drone has log variable to send from previous session, ignoring
		return;
	}

	bool alreadyHasVariable = logToc->getLogVariableIdForName(group + "." + name) != -1;
	if (!alreadyHasVariable)
	{

		DBG("Log Variable info received [" << currentLogVariableId << "] : " << group << "." << name << " (" << CFParam::getTypeString((CFParam::Type)type) << ") ");

		logToc->addVariableDef(group + "." + name, currentLogVariableId, (CFLogVariable::Type)type);
		currentLogVariableId = logToc->variables.size();

		DBG("Log toc is initialized ?" << (int)(logToc->isInitialized()) << " : " << logToc->numVariables << "< > " << logToc->variables.size());
		if (logToc->isInitialized())
		{
			logToc->save();
			addConnectionCommands(); // go to the next connection step
		} else
		{
			if (currentLogVariableId >= logToc->numVariables)
			{
				DBG("Problem variableId should not be more");
				return;
			}
			DBG("Add request for log item id " << currentLogVariableId);
			addCommand(CFCommand::createGetLogItemInfo(this, currentLogVariableId));
		}
	} else
	{
		DBG("Already has variable " << group << "." << name << ", skipping");
		currentLogVariableId++;
		DBG("Cur variable log id : " << currentLogVariableId);
		//DBG("Log toc is initialized ?" << (int)(logToc->isInitialized()) << " : " << logToc->numVariables << "< > " << logToc->variables.size());
		addCommand(CFCommand::createGetLogItemInfo(this, currentLogVariableId));
	}
}

void CFDrone::logBlockReceived(int blockId, var data)
{
	if (Engine::mainEngine->isClearing) return;
	if (CFSettings::getInstanceWithoutCreating() == nullptr) return;

	switch (blockId)
	{
	case LOG_POSITION_ID:
	{
		PosBlock * pLog = (PosBlock *)data.getBinaryData()->getData();

		//Invert Z and Y depending on zIsVertical option in Project Settings (if zIsVertical is not checked, then swap y and z because crazyflie consider z as vertical, so invert to reflect y as vertical in real pos)
		bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
		realPosition->setVector(pLog->x, zIsVertical ? pLog->y : pLog->z, zIsVertical ? pLog->z : pLog->y);
		orientation->setVector(pLog->rx, pLog->ry, pLog->rz);
		upsideDown->setValue(pLog->rz < -120 || pLog->rz > 120);
	}
	break;

	case LOG_POWER_ID:
	{
		BatteryBlock * bLog = (BatteryBlock *)data.getBinaryData()->getData();
		batteryLevel->setValue(bLog->battery / 100.0f);
		voltage->setValue(bLog->voltage);

		if (voltage->floatValue() < CFSettings::getInstance()->minBattery->floatValue())
		{
			if (!lowBattery->boolValue() && !MultiTimer::isTimerRunning(LOWBAT_ID))
			{
				MultiTimer::startTimer(LOWBAT_ID, 1000.0f * CFSettings::getInstance()->lowBatteryTime->floatValue());
				NLOG(niceName, "Entering Low battery check");
			}
		} else
		{
			stopTimer(LOWBAT_ID);
		}

		charging->setValue(bLog->charging > 0);
	}
	break;

	case LOG_CALIB_ID:
	{
		CalibBlock * cLog = (CalibBlock *)data.getBinaryData()->getData();

		DBG("Variance " << cLog->varianceX << ", " << cLog->varianceY << ", " << cLog->varianceZ);
		uint64 t = Time::getMillisecondCounter();
		bool stab = /*data->varianceX > 0 && data->varianceY > 0 && data->varianceZ > 0 &&*/ cLog->varianceX < minConvergeDist && cLog->varianceY < minConvergeDist && cLog->varianceZ < minConvergeDist;
		if (stab)
		{

			if (timeAtStartConverge == 0)
			{
				timeAtStartConverge = t;
				timeAtStartCalib = t;
			}

			if (t > timeAtStartConverge + minConvergeTime)
			{
				NLOG(niceName, "Calibrated");
				isCalibrated = true;
				addCommand(CFCommand::createStopLog(this, LOG_CALIB_ID));
				calibrationProgress->setValue(0);
				state->setValueWithData(READY);
			} else
			{
				calibrationProgress->setValue((float)((t - timeAtStartConverge)*1.0f / minConvergeTime));
			}
		} else
		{
			timeAtStartConverge = 0;
			isCalibrated = false; 
			calibrationProgress->setValue(0);
		}
	}
	break;
	}

}


void CFDrone::updateQualityPackets(bool val)
{
	bool oldVal = qualityPackets[qualityPacketIndex];
	if (oldVal != val)
	{
		qualityPackets[qualityPacketIndex] = val;

		float p = 0;
		for (int i = 0; i < maxQualityPackets; i++) p += (int)qualityPackets[i];
		linkQuality->setValue(p / maxQualityPackets);

	}
	qualityPacketIndex = (qualityPacketIndex + 1) % maxQualityPackets;

}

void CFDrone::stateChanged()
{
	DroneState s = state->getValueDataAsEnum<DroneState>();


	updateControls();

	stopAllTimers();

	if (!enabled->boolValue()) return;

	switch (s)
	{
	case POWERED_OFF:
		safeLinkActive = false;
		lowBattery->setValue(false);
		for (int i = 0; i < 10; i++) addCommand(CFCommand::createActivateSafeLink(this));
		startTimer(TIMER_PING);
		break;

	case DISCONNECTED:
	{
		//addCommand(CFCommand::createActivateSafeLink(this));
		if (CFSettings::getInstanceWithoutCreating() != nullptr && CFSettings::getInstance()->autoConnect->boolValue()) MultiTimer::startTimer(AUTOCONNECT_ID, 2500);
	}
	break;

	case CONNECTING:
	{
		if (!safeLinkActive) for (int i = 0; i < 10; i++) addCommand(CFCommand::createActivateSafeLink(this));

		currentParamRequestId = -1;
		currentLogVariableId = -1;
		addConnectionCommands();
	}
	break;

	case CALIBRATING:
	{
		isCalibrated = false;

		addParamCommand("kalman.resetEstimation", 1);
		addParamCommand("kalman.resetEstimation", 0);
		timeAtStartCalib = Time::getMillisecondCounter();
		timeAtStartConverge = 0;
		lightMode->setValueWithData(DOUBLE_SPINNER);

		//force those 2 to start as well if drone got disconnected
		addCommand(CFCommand::createStartLog(this, LOG_POWER_ID, 5));
		addCommand(CFCommand::createStartLog(this, LOG_POSITION_ID, 20));

		addCommand(CFCommand::createStartLog(this, LOG_CALIB_ID, 20));

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
		for (int i = 0; i < 20; i++) addCommand(CFCommand::createSetPoint(this, 0, 0, 0, 0)); //unlock thrust commands
		startTimer(TIMER_TAKEOFF);
	}
	break;

	case FLYING:
	{
		Vector3D<float> target = realPosition->getVector();
		target.y = takeOffHeight->floatValue();
		targetPosition->setVector(target);
		desiredPosition->setVector(target);
		startTimer(TIMER_FLYING);
	}
	break;

	case LANDING:
	{
		timeAtStartLanding = Time::getMillisecondCounter();
		float h = CFSettings::getInstance()->zIsVertical->boolValue() ? realPosition->z : realPosition->y;
		landingTime = jmax<float>(h * 1.25f, 2);
		NLOG(niceName, "Landing for " << landingTime << "seconds.");
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
	switch (timerID)
	{

	case TIMER_PING:
	{
		addCommand(CFCommand::createPing(this));
	}
	break;

	case TIMER_TAKEOFF:
	{
		syncToRealPosition();
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
			addCommand(CFCommand::createStopLog(this, LOG_CALIB_ID));
			state->setValueWithData(WARNING);
		}
	}
	break;

	case LOWBAT_ID:
		color->setColor(Colours::red);
		fadeTime->setValue(1.5f);
		headlight->setValue(false);
		lowBattery->setValue(true);
		stopTimer(LOWBAT_ID);
		NLOGWARNING(niceName, "LOW BATTERY");
		break;

	case LOWBAT_BLINK_ID:
		color->setColor(color->getColor() == Colours::black ? Colours::red.darker() : Colours::black);
		addParamCommand("ring.fadeColor", (int64)color->getColor().getARGB());
		break;

	case AUTOCONNECT_ID:
		stopTimer(AUTOCONNECT_ID);
		state->setValueWithData(CONNECTING);
		break;

	case UPSIDE_DOWN_ID:
		stopTimer(UPSIDE_DOWN_ID);
		addCommand(CFCommand::createStop(this));
		state->setValueWithData(WARNING);
		lightMode->setValueWithData(LightMode::OFF);
		break;

	default:
		//not handled
		break;
	}
}
