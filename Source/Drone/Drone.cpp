/*
  ==============================================================================

	Drone.cpp
	Created: 25 Nov 2019 2:38:27pm
	Author:  bkupe

  ==============================================================================
*/

#include "Drone.h"
#include "Engine/CFSettings.h"

Drone::Drone() :
	BaseItem("0"),
	Thread("Drone"),
	safeLinkActive(false),
	safeLinkUpFlag(false),
	safeLinkDownFlag(false),
	infoCC("Infos"),
	controlCC("Controls"),
	flightCC("Flight"),
	lightsCC("Lights"),
	decksCC("Custom Decks"),
	logToc(nullptr),
	paramToc(nullptr),
	numMemories(0),
	lhMemoryId(0),
	curMemoryId(0),
	pidCC("PID Override")
{

	itemDataType = "Drone";

	nameCanBeChangedByUser = false;

	id = infoCC.addIntParameter("ID", "ID of the drone, determining address and channel", 0, 0, 50);
	state = infoCC.addEnumParameter("State", "State of the drone");
	for (int i = 0; i < STATE_MAX; i++) state->addOption(stateNames[i], (State)i);
	state->setControllableFeedbackOnly(true);
	battery = infoCC.addFloatParameter("Battery", "Normalized battery level", 0);
	battery->setControllableFeedbackOnly(true);
	linkQuality = infoCC.addFloatParameter("Link Quality", "Radio link qualiry", 0, 0, 1);
	linkQuality->setControllableFeedbackOnly(true);
	addChildControllableContainer(&infoCC);

	takeOffTrigger = controlCC.addTrigger("Take off", "Take off");
	landTrigger = controlCC.addTrigger("Land","Land");
	stopTrigger = controlCC.addTrigger("Stop", "Stop");
	rebootTrigger = controlCC.addTrigger("Reboot", "Reboot");
	onTrigger = controlCC.addTrigger("System On", "ON");
	offTrigger = controlCC.addTrigger("System Off", "OFF");
	addChildControllableContainer(&controlCC);

	//flightSmoothing = flightCC.addFloatParameter("Flight Smoothing", "Time it takes when setting position", 0, 0);
	desiredPosition = flightCC.addPoint3DParameter("Desired Position", "The target position to send to the drone");
	desiredSpeed = flightCC.addPoint3DParameter("Desired Speed", "The target position to send to the drone");
	desiredSpeed->hideInEditor = true;
	desiredAcceleration = flightCC.addPoint3DParameter("Desired Speed", "The target position to send to the drone");
	desiredAcceleration->hideInEditor = true; 
	
	targetYaw = flightCC.addFloatParameter("Target Yaw", "The target horizontal rotation to send to the drone", 0, -1, 1);
	
	enableYawLookAt = flightCC.addBoolParameter("LookAt Mode", "The target horizontal rotation to send to the drone", false);
	yawLookAt = flightCC.addPoint2DParameter("LookAt Position", "The target horizontal rotation to send to the drone");

	realPosition = flightCC.addPoint3DParameter("Real Position", "The tracked position sent from the drone");
	realPosition->setControllableFeedbackOnly(true);
	realRotation = flightCC.addPoint3DParameter("Real Rotation", "The tracked rotation sent from the drone");
	realRotation->setControllableFeedbackOnly(true);
	
	pidCC.enabled->setValue(false);
	attitude_rollPitch_Kp = pidCC.addFloatParameter("Attitude RollPitch Kp", "P of attitude roll and pitch value", 3, 0);
	attitude_rollPitch_Ki = pidCC.addFloatParameter("Attitude RollPitch Ki", "I of attitude roll and pitch value", 6, 0);
	attitude_rollPitch_Kd = pidCC.addFloatParameter("Attitude RollPitch Kd", "D of attitude roll and pitch value", 0, 0);
	rate_rollPitch_Kp = pidCC.addFloatParameter("Attitude RollPitch Kp", "P of rate roll and pitch value", 250, 0);
	rate_rollPitch_Ki = pidCC.addFloatParameter("Attitude RollPitch Ki", "I of rate roll and pitch value", 500, 0);
	rate_rollPitch_Kd = pidCC.addFloatParameter("Attitude RollPitch Kd", "D of rate roll and pitch value", 2.5f, 0);

	flightCC.addChildControllableContainer(&pidCC);

	addChildControllableContainer(&flightCC);

	lightMode = lightsCC.addEnumParameter("LightMode", "Led Preset");
	for (int i = 0; i < LIGHTMODE_MAX; i++) lightMode->addOption(lightModeNames[i], (LightMode)i);
	lightMode->setValueWithData(FADE_COLOR);

	color = lightsCC.addColorParameter("Light Color", "The color of the led ring. Only works in fade color mode", Colours::black);
	fadeTime = lightsCC.addFloatParameter("Fade time", "The time to fade from one color to another", .1f, 0, 10);
	headlight = lightsCC.addBoolParameter("Headlight", "Headlight", false);
	stealthMode = lightsCC.addBoolParameter("Stealth Mode", "When in stealthMode, the system leds are off", false);
	addChildControllableContainer(&lightsCC);

	updateCustomDecks();
	addChildControllableContainer(&decksCC);

	viewUISize->setPoint(50, 50);

	startThread();
}

Drone::~Drone()
{

	disconnect();
	
	signalThreadShouldExit();
	waitForThreadToExit(3000);

	masterReference.clear();
}

void Drone::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == enabled)
	{
		if (enabled->boolValue())
		{
			startThread();
		}
		else
		{
			threadShouldExit();
			waitForThreadToExit(2000);
			state->setValueWithData(DISCONNECTED);
		}
	}
	else if (p == viewUIPosition)
	{
		if (viewUIPosition->x != desiredPosition->x && viewUIPosition->y != desiredPosition->z)
		{
			desiredPosition->setVector(viewUIPosition->x/100, desiredPosition->y, -viewUIPosition->y/100);
		}
	}

	BaseItem::onContainerParameterChangedInternal(p);
}

void Drone::onControllableFeedbackUpdateInternal(ControllableContainer*, Controllable* c)
{
	if (!enabled->boolValue()) return;

	if (c == id)
	{
		setNiceName(id->stringValue());
		disconnect();
	}

	else if (c == takeOffTrigger) takeOff();
	else if (c == landTrigger) land();
	else if (c == stopTrigger) stop();
	else if (c == rebootTrigger) reboot();
	else if (c == onTrigger) setSystemPower(false);
	else if (c == offTrigger) setSystemPower(true);

	else if (c == desiredPosition || c == targetYaw || c == enableYawLookAt ||c == yawLookAt)
	{
		//float distFromRealPos = (desiredPosition->getVector() - realPosition->getVector()).length();
		sendPosition();
	}
	else if (c == realPosition)
	{
		if (state->getValueDataAsEnum<State>() == READY && !isFlying())
		{
			desiredPosition->setVector(realPosition->getVector());
		}
	}

	else if (c == color) setParam("ring.fadeColor", (int64)color->getColor().getARGB());
	else if (c == headlight) setParam("ring.headlightEnable", headlight->boolValue());
	else if (c == lightMode) setParam("ring.effect", lightMode->getValueData());
	else if (c == fadeTime) setParam("ring.fadeTime", fadeTime->floatValue());
	else if (c == stealthMode) setParam("platform.stealthMode", stealthMode->boolValue());

	else if (c->parentContainer->parentContainer == &decksCC)
	{
		Parameter* p = dynamic_cast<Parameter*>(c);
		if (p != nullptr)
		{
			switch (p->type)
			{
			case Controllable::COLOR: setParam(c->niceName, (int64)((ColorParameter*)p)->getColor().getARGB());
			case Controllable::FLOAT: setParam(c->niceName, p->floatValue());
			case Controllable::INT: setParam(c->niceName, p->intValue());
			case Controllable::BOOL: setParam(c->niceName, p->boolValue());
			}
		}
	}
	else if (c->parentContainer == &pidCC)
	{
		if (pidCC.enabled->boolValue())
		{

			FloatParameter* p = (FloatParameter*)c;

			if (c == attitude_rollPitch_Kp)
			{
				setParam("pid_attitude.pitch_kp", p->floatValue());
				setParam("pid_attitude.roll_kp", p->floatValue());
			}else if(c == attitude_rollPitch_Ki)
			{
				setParam("pid_attitude.pitch_ki", p->floatValue());
				setParam("pid_attitude.roll_ki", p->floatValue());
			}
			else if (c == attitude_rollPitch_Kd)
			{
				setParam("pid_attitude.pitch_kd", p->floatValue());
				setParam("pid_attitude.roll_kd", p->floatValue());
			}
			else if (c == rate_rollPitch_Kp)
			{
				setParam("pid_rate.pitch_kp", p->floatValue());
				setParam("pid_rate.roll_kp", p->floatValue());
			}
			else if (c == rate_rollPitch_Ki)
			{
				setParam("pid_rate.pitch_ki", p->floatValue());
				setParam("pid_rate.roll_ki", p->floatValue());
			}
			else if (c == rate_rollPitch_Kd)
			{
				setParam("pid_rate.pitch_kd", p->floatValue());
				setParam("pid_rate.roll_kd", p->floatValue());
			}
		}
	}
}

void Drone::ping()
{
	if (!enabled->boolValue()) return;
	addCommand(CFCommand::createPing(this));
}

void Drone::takeOff()
{
	if (!enabled->boolValue()) return;
	if (!canFly())
	{
		NLOGWARNING(niceName, "Drone can't fly, not taking off");
		return;
	}

	if (isFlying())
	{
		NLOGWARNING(niceName, "Drone already flying, not taking off");
		return;
	}

	float targetHeight = CFSettings::getInstance()->takeOffHeight->floatValue();
	float targetTime = CFSettings::getInstance()->takeOffTime->floatValue();

	addCommand(CFCommand::createHighLevelTakeOff(this, targetHeight, targetTime));

	//synchronize with expected position after takeOff
	desiredPosition->setVector(realPosition->x, targetHeight, realPosition->z);
	targetYaw->setValue(0);

//	targetPosition->setVector(realPosition->x, targetHeight, realPosition->z);

	state->setValueWithData(TAKING_OFF);
	startTimer(targetTime * 1000);
}

void Drone::land()
{
	if (!isFlying())
	{
		NLOGWARNING(niceName, "Drone is not flying not landing");
		return;
	}

	float targetTime = jmax<float>(realPosition->y, 3);
	addCommand(CFCommand::createHighLevelLand(this, 0, targetTime), true);

	state->setValueWithData(LANDING);
	startTimer(targetTime * 1000);
}

void Drone::stop()
{
	if (state->getValueDataAsEnum<State>() == DISCONNECTED)
	{
		NLOGWARNING(niceName, "Drone is not connected, not sending stop");
		return;
	}
	
	addCommand(CFCommand::createStop(this), true);
	state->setValueWithData(DISCONNECTED);
}

void Drone::reboot()
{
	if (!enabled->boolValue()) return;
	if (isFlying())
	{
		NLOG(niceName, "Drone is flying, not rebooting");
		return;
	}

	addCommand(CFCommand::createRebootInit(this));
	addCommand(CFCommand::createRebootFirmware(this));
	state->setValueWithData(DISCONNECTED);
}

void Drone::setSystemPower(bool power)
{
	if (power)
	{
		addCommand(CFCommand::createSystemOn(this));
		state->setValueWithData(DISCONNECTED);
	}
	else addCommand(CFCommand::createSystemOff(this));

}

void Drone::setSystemLed(bool isOn)
{
	addCommand(CFCommand::createSystemLed(this, isOn));
}

void Drone::setParam(StringRef name, var value)
{
	if (!enabled->boolValue()) return;
	if (state->getValueDataAsEnum<State>() == DISCONNECTED)
	{
		NLOGWARNING(niceName, "Drone is not connected, not sending stop");
		return;
	}
	if (paramToc == nullptr)
	{
		NLOGWARNING(niceName, "Param TOC is not initialized yet.");
		return;
	}
	else if(paramToc->getParamIdForName(name) == -1)
	{
		NLOGWARNING(niceName, "Parameter " << name << "doesn't exist !");
		return;
	}
	addUniqueCommand(CFCommand::createSetParam(this, String(name), value));
}

void Drone::setPosition(Vector3D<float> position, float yaw, float time)
{
	if (state->getValueDataAsEnum<State>() != FLYING) return;
	addCommand(CFCommand::createHighLevelGoto(this, CFSettings::toDroneVector(position), yaw, time));
}


bool Drone::canFly()
{
	State s = state->getValueDataAsEnum<State>();
	return s == READY || s == TAKING_OFF || s == FLYING || s == LANDING;
}

bool Drone::isFlying()
{
	State s = state->getValueDataAsEnum<State>();
	return s == TAKING_OFF || s == FLYING || s == LANDING;
}

void Drone::updateCustomDecks()
{
	decksCC.clear();

	for (auto& dc : CFSettings::getInstance()->deckManager.items)
	{
		ControllableContainer* dcc = new ControllableContainer(dc->niceName);
		for (auto& dcp : dc->paramManager.items)
		{
			Parameter* c = ControllableFactory::createParameterFrom((Parameter*)(dcp->controllable));
			c->setNiceName(dcp->niceName);
			dcc->addParameter(c);
		}

		decksCC.addChildControllableContainer(dcc, true);
	}
}

void Drone::addCommand(CFCommand * command, bool force)
{
	if (!enabled->boolValue()) return;

	commands.getLock().enter();

	bool found = false;
	for (int i = 0; i < commands.size(); i++)
	{
		if (commands[i]->type == command->type)
		{
			delete(commands[i]);
			commands.set(i, command);
			found = true;
			break;
		}
	}

	if (!found) commands.add(command);

	commands.getLock().exit();
}

void Drone::addUniqueCommand(CFCommand * command)
{
	if (!enabled->boolValue()) return;

	uniqueCommands.getLock().enter();
	uniqueCommands.add(command);
	uniqueCommands.getLock().exit();
}


void Drone::clearCommands()
{
	commands.getLock().enter();
	while (commands.size() > 0)  delete commands.removeAndReturn(0);
	commands.getLock().exit();

	uniqueCommands.getLock().enter();
	while (commands.size() > 0)  delete uniqueCommands.removeAndReturn(0);
	uniqueCommands.getLock().exit();
}

void Drone::connect()
{
	if (isClearing || Engine::mainEngine->isClearing) return;

}

void Drone::setupDrone()
{
	if (isClearing) return;
	
	NLOG(niceName, "Connecting...");
	for (int i = 0; i < 10; i++) addCommand(CFCommand::createActivateSafeLink(this));

	for (int i = 0; i < 200; i++)
	{
		if (threadShouldExit() || state->getValueDataAsEnum<State>() != CONNECTING) return; //avoid blocking for 1.5 seconds
		sleep(10);
	}

	NLOG(niceName, "Synchronizing parameters...");

	addCommand(CFCommand::createRequestParamToc(this));

	while (paramToc == nullptr)
	{
		if (threadShouldExit() || state->getValueDataAsEnum<State>() != CONNECTING) return; 
		ping();
		sleep(10);
	}

	NLOG(niceName, paramToc->numParams << " parameters to synchronize");
	int lastParamId = -1;
	while (!paramToc->isInitialized())
	{
		if (threadShouldExit() || state->getValueDataAsEnum<State>() != CONNECTING) return;
		int paramId = paramToc->getMissingIds()[0];
		if (lastParamId != paramId)
		{
			NLOG(niceName, "Synchronizing  parameter " << paramId << " ...");
			addCommand(CFCommand::createGetParamInfo(this, paramId));
			lastParamId = paramId;
		}
		else
		{
			ping();
		}
		sleep(10);
	}

	paramToc->save();
	NLOG(niceName, "Parameters are synchronized and saved");

	NLOG(niceName, "Synchronizing log variables...");
	addCommand(CFCommand::createRequestLogToc(this));
	
	while (logToc == nullptr)
	{
		if (threadShouldExit() || state->getValueDataAsEnum<State>() != CONNECTING) return;
		ping();
		sleep(2);
	}


	NLOG(niceName, logToc->numVariables << " log variables to synchronize");
	
	int lastLogId = -1;
	while (!logToc->isInitialized())
	{
		if (threadShouldExit() || state->getValueDataAsEnum<State>() != CONNECTING) return;
		int logId = logToc->getMissingIds()[0];
		if (lastLogId != logId)
		{
			NLOG(niceName, "Synchronizing  log variable " << logId << " ...");
			addCommand(CFCommand::createGetLogItemInfo(this, logId));
			lastLogId = logId;
		}
		else
		{
			ping();
		}
		sleep(2);
	}

	logToc->save();
	NLOG(niceName, "Log variables are synchronized and saved");


	NLOG(niceName, "Getting memory info...");

	addCommand(CFCommand::createGetMemoryNumber(this));

	while (numMemories == 0)
	{
		if (threadShouldExit() || state->getValueDataAsEnum<State>() != CONNECTING) return;

		ping();
		sleep(2);
	}

	NLOG(niceName,"Found " << numMemories << ", getting infos");

	memories.clear();

	int lastMemoryId = -1;
	while (memories.size() < numMemories)
	{
		if (threadShouldExit() || state->getValueDataAsEnum<State>() != CONNECTING) return;
		if (lastMemoryId != curMemoryId)
		{
			LOG("Requesting info for id : " << curMemoryId);
			addCommand(CFCommand::createGetMemoryInfo(this, curMemoryId));
			lastMemoryId = curMemoryId;
		}
		else
		{
			ping();
			sleep(2);
		}
	}

	NLOG(niceName, "Enable high level commander");
	setParam("commander.enHighLevel", 1);

	int controllerId = CFSettings::getInstance()->flightController->getValueDataAsEnum<CFSettings::FlightController>();
	NLOG(niceName, "Use " << CFSettings::getInstance()->controllerNames[(int)controllerId] << " controller");
	setParam("stabilizer.controller", controllerId); //0 = auto, 1 = pid, 2 = mellinger

	if (controllerId == 1 && pidCC.enabled->boolValue()) // pid
	{
		setParam("pid_attitude.pitch_kp", attitude_rollPitch_Kp->floatValue());
		setParam("pid_attitude.roll_kp", attitude_rollPitch_Kp->floatValue());
		setParam("pid_attitude.pitch_ki", attitude_rollPitch_Ki->floatValue());
		setParam("pid_attitude.roll_ki", attitude_rollPitch_Ki->floatValue());
		setParam("pid_attitude.pitch_kd", attitude_rollPitch_Kd->floatValue());
		setParam("pid_attitude.roll_kd", attitude_rollPitch_Kd->floatValue());
		setParam("pid_rate.pitch_kp", rate_rollPitch_Kp->floatValue());
		setParam("pid_rate.roll_kp", rate_rollPitch_Kp->floatValue());
		setParam("pid_rate.pitch_ki", rate_rollPitch_Kp->floatValue());
		setParam("pid_rate.roll_ki", rate_rollPitch_Kp->floatValue());
		setParam("pid_rate.pitch_kd", rate_rollPitch_Kp->floatValue());
		setParam("pid_rate.roll_kd", rate_rollPitch_Kp->floatValue());
	}

	NLOG(niceName, "Set light deck");
	setParam("ring.fadeColor", (int64)color->getColor().getARGB());
	setParam("ring.headlightEnable", headlight->boolValue());
	setParam("ring.effect", lightMode->getValueData());
	setParam("ring.fadeTime", fadeTime->floatValue());

	// LOG BLOCKS
	NLOG(niceName, "Registering logs...");
	addCommand(CFCommand::createResetLogs(this));

	addCommand(CFCommand::createAddLogBlock(this, 2, Array<String>("pm.batteryLevel", "pm.vbat", "pm.state")));
	addCommand(CFCommand::createAddLogBlock(this, 1, Array<String>("stateEstimate.x", "stateEstimate.y", "stateEstimate.z", "stabilizer.pitch", "stabilizer.yaw","stabilizer.roll")));
	
	addCommand(CFCommand::createStartLog(this, 2, 5));
	addCommand(CFCommand::createStartLog(this, 1, 20));


	if (CFSettings::getInstance()->lighthouseCC.enabled->boolValue()) setLighthouseSetup();
	
	NLOG(niceName, "Drone is ready !");
	state->setValueWithData(READY);
}

void Drone::disconnect()
{
	if (state->getValueDataAsEnum<State>() != DISCONNECTED)
	{
		NLOGWARNING(niceName, "Disconnecting drone");
		state->setValueWithData(DISCONNECTED);
		
		clearCommands();

		safeLinkActive = false;
	}

}

void Drone::setLighthouseSetup()
{
	if (Engine::mainEngine->isClearing) return;

	Crazyflie::LighthouseBSGeometry bs1;
	Crazyflie::LighthouseBSGeometry bs2;
	//cf->getLighthouseGeometries(bs1, bs2);

	//Vector3D<float> bs1Pos(bs1.origin[0], bs1.origin[1], bs1.origin[2]);
	//Vector3D<float> bs2Pos(bs2.origin[0], bs2.origin[1], bs2.origin[2]);

	//NLOG(niceName, "Lighthouse GET geometries : " << bs1Pos.x << ", " << bs1Pos.y << ", " << bs1Pos.z << " / " << bs2Pos.x << ", " << bs2Pos.y << ", " << bs2Pos.z);


	Vector3D<float> bs1Origin = CFSettings::getInstance()->bs1Origin->getVector();
	Vector3D<float> bs2Origin = CFSettings::getInstance()->bs2Origin->getVector();

	for (int i = 0; i < 3; i++)
	{
		bs1.origin[i] = CFSettings::getInstance()->bs1Origin->value[i];
		bs2.origin[i] = CFSettings::getInstance()->bs2Origin->value[i];

		for (int j = 0; j < 3; j++)
		{
			bs1.matrix[i][j] = CFSettings::getInstance()->bs1MatRows[i]->value[j];
			bs2.matrix[i][j] = CFSettings::getInstance()->bs2MatRows[i]->value[j];
		}
	}


	//Set geometry
	uint8_t data[96]; // 2 bs struct
	memcpy(data, reinterpret_cast<const uint8_t*>(&bs1), sizeof(Crazyflie::LighthouseBSGeometry));
	memcpy(data + sizeof(Crazyflie::LighthouseBSGeometry), reinterpret_cast<const uint8_t*>(&bs2), sizeof(Crazyflie::LighthouseBSGeometry));
	
	Array<uint8_t> dataArray;
	dataArray.addArray(data, 96);

	int lastMemAddress = -1;
	curLHMemoryAddress = 0;
	while(dataArray.size() > 0)
	{
		if (lastMemAddress != curLHMemoryAddress)
		{
			//LOG("Add write command at address : " << curLHMemoryAddress);
			addCommand(CFCommand::createWriteMemory(this, lhMemoryId, curLHMemoryAddress, dataArray));
			dataArray.removeRange(0, 24);
			lastMemAddress = curLHMemoryAddress;
		}
		else
		{
			ping();
			sleep(2);
		}
	}

	
	lhMemoryData.clear();
	lastMemAddress = -1;
	curLHMemoryAddress = 0;
	while (curLHMemoryAddress < 96)
	{
		if (lastMemAddress != curLHMemoryAddress)
		{
			//LOG("Add read command at address : " << curLHMemoryAddress);
			addCommand(CFCommand::createReadMemory(this, lhMemoryId, curLHMemoryAddress, jmin(24, 96 - curLHMemoryAddress)));
			dataArray.removeRange(0, 24);
			lastMemAddress = curLHMemoryAddress;
		}
		else
		{
			ping();
			sleep(2);
		}
	}


	int bsDataLen = sizeof(Crazyflie::LighthouseBSGeometry);
	memcpy(&bs1, lhMemoryData.getRawDataPointer(), bsDataLen);
	memcpy(&bs2, lhMemoryData.getRawDataPointer() + bsDataLen, bsDataLen);

	
	Vector3D<float> bs1Pos = Vector3D<float>(bs1.origin[0], bs1.origin[1], bs1.origin[2]);
	Vector3D<float> bs2Pos = Vector3D<float>(bs2.origin[0], bs2.origin[1], bs2.origin[2]);

	NLOG(niceName, "Lighthouse positions check : " << bs1Pos.x << ", " << bs1Pos.y << ", " << bs1Pos.z << " / " << bs2Pos.x << ", " << bs2Pos.y << ", " << bs2Pos.z);
	/*for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			LOG(" > " << bs1.matrix[i][j]);
		}
	}*/
}

String Drone::getURI() const
{
	int targetRadio = floor(id->intValue() / 10); //10 drones / radio
	int targetChannel = id->intValue() * 2;
	String droneAddress = "E7E7E7E7" + String::formatted("%02d", id->intValue());
	return "radio://" + String(targetRadio) + "/" + String(targetChannel) + "/2M/" + droneAddress;
}

void Drone::sendPosition()
{
	float tYaw = getDesiredYaw();
	setPosition(desiredPosition->getVector(), tYaw, 0);// flightSmoothing->floatValue());
	viewUIPosition->setPoint(desiredPosition->getVector().x * 100, -desiredPosition->getVector().z * 100);
}

float Drone::getDesiredYaw()
{
	float tYaw = targetYaw->floatValue() * float_Pi * 2;

	if (enableYawLookAt->boolValue())
	{
		Point<float> posXZ(desiredPosition->x, desiredPosition->z);
		Point<float> lookAtXZ = yawLookAt->getPoint();
		float lookAtYaw = posXZ.getAngleToPoint(lookAtXZ) - float_Pi / 2;
		tYaw += lookAtYaw;
	}

	return tYaw;
}

void Drone::noAckReceived()
{
	const int maxTimeoutPackets = 100;
	linkQuality->setValue(linkQuality->floatValue() - 1.0f / maxTimeoutPackets);
	if (linkQuality->floatValue() == 0) disconnect();
}

void Drone::packetReceived(CFPacket * packet)
{
	//if(packet.type != CFPacket::RSSI_ACK) NLOG(niceName, "Packet received : " << packet.getTypeString());
	
	State s = state->getValueDataAsEnum<State>();
	if (s == DISCONNECTED)
	{
		linkQuality->setValue(1);
		state->setValueWithData(CONNECTING);
	}
	else
	{
		const int maxTimeoutPackets = 100;
		linkQuality->setValue(linkQuality->floatValue() + 1.0f / maxTimeoutPackets);
	}

	switch (packet->type)
	{
	case CFPacket::CONSOLE: consolePacketReceived(packet->data.toString()); break;
	case CFPacket::RSSI_ACK: rssiAckReceived((int)packet->data); break;
	
	case CFPacket::PARAM_TOC_INFO: paramTocReceived((int)packet->data.getProperty("crc", 0), (int)packet->data.getProperty("size", 0)); break;
	case CFPacket::PARAM_TOC_ITEM: paramInfoReceived(
		(int)packet->data.getProperty("id", -1),
		packet->data.getProperty("group", "[notset]").toString(),
		packet->data.getProperty("name", "[notset]").toString(),
		(int)packet->data.getProperty("type", -1),
		(bool)packet->data.getProperty("readOnly", 0),
		(int)packet->data.getProperty("length", -1),
		(int)packet->data.getProperty("sign", -1));
		break;
		
	case CFPacket::LOG_TOC_INFO: logTocReceived((int)packet->data.getProperty("crc", 0), (int)packet->data.getProperty("size", 0)); break;
	case CFPacket::LOG_TOC_ITEM: logVariableInfoReceived(
		(int)packet->data.getProperty("id", -1),
		packet->data.getProperty("group", "[notset]").toString(),
		packet->data.getProperty("name", "[notset]").toString(),
		(int)packet->data.getProperty("type", -1));
		break;

	case CFPacket::LOG_DATA:
		logDataReceived(packet->data.getProperty("blockId", -1), packet->data.getProperty("data", var()));
		break;

	case CFPacket::LOG_CONTROL:
		DBG("Got log control here");
		break;

	case CFPacket::PARAM_VALUE:
		DBG("Got Param value : " << packet->data.getProperty("name","[notset]").toString() << " : " << packet->data.getProperty("value","[notset]").toString());
		break;

	case CFPacket::MEMORY_NUMBER:
		numMemories = (int)packet->data;
		break;

	case CFPacket::MEMORY_INFO:
		memoryInfoReceived((uint64)(int64)packet->data.getProperty("address",0), (Crazyflie::MemoryType)(int)packet->data.getProperty("type",0), packet->data.getProperty("size", 0));
		break;

	case CFPacket::MEMORY_READ:
		memoryReadReceived((int)packet->data.getProperty("memoryId", -1), (uint64)(int64)packet->data.getProperty("address", 0), (int)packet->data.getProperty("status", -1), packet->data.getProperty("data",var()));
		break;

	case CFPacket::MEMORY_WRITE:
		memoryWriteReceived((int)packet->data.getProperty("memoryId", -1), (uint64)(int64)packet->data.getProperty("address", 0), (int)packet->data.getProperty("status", -1));
		break;

	default:
		LOG("Not handled : " << packet->getTypeString());
		break;
	}
}

void Drone::rssiAckReceived(int data)
{
}

void Drone::paramTocReceived(int crc, int size)
{
	paramToc = CFParamToc::getParamToc(crc);
	if (paramToc == nullptr)
	{
		LOG("Toc doesn't exist, create a new one");
		paramToc = CFParamToc::addParamToc(crc, size);
	}
	else
	{
		LOG("Toc already exists, is init ? " << (int)paramToc->isInitialized());
	}
}

void Drone::paramInfoReceived(int paramId, String group, String name, int type, bool readOnly, int length, int sign)
{
	if (paramToc == nullptr) return;
	paramToc->addParamDef(group + "." + name, paramId, (CFParam::Type)type);
}

void Drone::logTocReceived(int crc, int size)
{
	logToc = CFLogToc::getLogToc(crc);
	if (logToc == nullptr)
	{
		logToc = CFLogToc::addLogToc(crc, size);
	}
	else
	{
		LOG("Toc already exists, is init ? " << (int)logToc->isInitialized());
	}
}

void Drone::logVariableInfoReceived(int logVariableId, String group, String name, int type)
{
	if (logToc == nullptr) return;
	logToc->addVariableDef(group + "." + name, logVariableId, (CFLogVariable::Type)type);
}

void Drone::logDataReceived(int blockId, var data)
{
	
	
	switch (blockId)
	{
	case 0:
	{
		BatteryBlock* bLog = (BatteryBlock*)data.getBinaryData()->getData();
		DBG("Battery : " << bLog->battery << ", " << bLog->voltage << ", " << bLog->charging);
		battery->setValue(bLog->battery);
	}
		break;

	case 1:
	{
		PosBlock* pLog = (PosBlock*)data.getBinaryData()->getData();
		realPosition->setVector(CFSettings::toLMFVector(Vector3D<float>(pLog->x, pLog->y, pLog->z)));
		realRotation->setVector(CFSettings::toLMFVector(Vector3D<float>(pLog->rx, pLog->ry, pLog->rz)));
	}
		break;
	}
}

void Drone::memoryInfoReceived(uint64 memoryAddress, Crazyflie::MemoryType type, int size)
{
	memories.add({ (uint16_t)curMemoryId, type, (uint32_t)size, memoryAddress });
	if (type == Crazyflie::MemoryTypeLH)
	{
		lhMemoryId = curMemoryId;
		//lhMemoryAddress = memoryAddress;
		LOG("Found LH Memory : " << lhMemoryId);
	}

	curMemoryId++;
}

void Drone::memoryReadReceived(int _id, uint64 memoryAddress, int status, var data)
{
	//LOG("Read received : " << id << ", " << memoryAddress << " : " << status << " > " << data.size());
	if (_id == lhMemoryId)
	{
		curLHMemoryAddress += data.size();
	}

	for (int i = 0; i < data.size(); i++)
	{
		lhMemoryData.add((int)data[i]);
	}
}

void Drone::memoryWriteReceived(int id, uint64 memoryAddress, int status)
{
	//LOG("Write received : " << id << ", " << memoryAddress << " : " << status);
	if (id == lhMemoryId)
	{
		curLHMemoryAddress += 24;
	}
}

void Drone::consolePacketReceived(const String &data)
{
	consoleBuffer += data;
	if (String(data).containsChar('\n'))
	{
		String lc = consoleBuffer.toLowerCase();
		if (lc.contains("fail")) NLOGERROR(niceName, consoleBuffer);
		else if (lc.contains("warning")) NLOGWARNING(niceName, consoleBuffer);
		else NLOG(niceName, consoleBuffer); consoleBuffer.clear();
	}
}

void Drone::run()
{
	while (!threadShouldExit())
	{
		while ((!isClearing && !threadShouldExit()) && state->getValueDataAsEnum<State>() == DISCONNECTED)
		{
			ping();
			for (int i = 0; i < 50 && !threadShouldExit(); i++) sleep(10);
		}

		while (state->getValueDataAsEnum<State>() == CONNECTING && !threadShouldExit())
		{
			LOG("Setup here");
			setupDrone();
			LOG("Setup finished");
		}
		
		
		if (state->getValueDataAsEnum<State>() == FLYING)
		{
			sendPosition();
		}

		while (!threadShouldExit())
		{
			State s = state->getValueDataAsEnum<State>();
			switch (s)
			{
			case READY:
			case TAKING_OFF:
			case LANDING:
				break;

			case FLYING:
				break;

			case WARNING:
				break;
			}

			if (commands.size() == 0 && uniqueCommands.size() == 0)
			{
				ping();
			}

			if (s == DISCONNECTED || s == CONNECTING)
			{
				break;
			}

			sleep(20); //50fps
		}
	}

	NLOGERROR(niceName, "Exit thread");
}


void Drone::timerCallback()
{
	if (!enabled->boolValue())
	{
		state->setValueWithData(DISCONNECTED);
	}
	else
	{
		State s = state->getValueDataAsEnum<State>();
		if (s == TAKING_OFF)
		{
			state->setValueWithData(FLYING);
		}
		else if (s == LANDING)
		{
			state->setValueWithData(READY);
		}
	}
	
	stopTimer();
}

var Drone::getJSONData()
{
	var data = BaseItem::getJSONData();
	data.getDynamicObject()->setProperty("droneId", id->intValue());
	return data;
}

void Drone::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	id->setValue(data.getProperty("droneId", id->intValue()));

	updateCustomDecks();
}
