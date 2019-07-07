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
	Thread("Drone"),
	safeLinkActive(false),
	safeLinkDownFlag(false),
	safeLinkUpFlag(false),
	qualityPacketIndex(0),

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
	timeAtStartHealthCheck(0),
	lastPhysicsUpdateTime(0)
{
	nameCanBeChangedByUser = false;

	batteryBlockCallback = std::bind(&CFDrone::batteryBlockReceived, this, std::placeholders::_1, std::placeholders::_2);
	posBlockCallback = std::bind(&CFDrone::posBlockReceived, this, std::placeholders::_1, std::placeholders::_2);
	calibBlockCallback = std::bind(&CFDrone::calibBlockReceived, this, std::placeholders::_1, std::placeholders::_2);

	emptyAckFunc = std::bind(&CFDrone::rssiAckReceived, this, std::placeholders::_1);
	linkQualityFunc = std::bind(&CFDrone::linkQualityReceived, this, std::placeholders::_1);
	consoleFunc = std::bind(&CFDrone::consolePacketReceived, this, std::placeholders::_1);

	droneId = addIntParameter("ID", "Id of the drone, auto-defining channel and address.\nId of 0 will be adress 0xE7E7E7E700 and channel 0. Id of 1 will be 0xE7E7E7E701 and channel 2", 0, 0, 60);

	addChildControllableContainer(&statusCC);
	state = statusCC.addEnumParameter("Drone State", "State of the drone");
	state->addOption("Disconnected", DISCONNECTED)
		->addOption("Calibrating", CALIBRATING)->addOption("Health Check", HEALTH_CHECK)
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
		BoolParameter* dc = decksCC.addBoolParameter("Deck " + deckNames[i], "Is the deck " + deckIds[i] + " connected ?", false);
		deckMap.set(deckIds[i], dc);
	}

	for (auto& c : decksCC.controllables) c->setControllableFeedbackOnly(true);

	statusCC.addChildControllableContainer(&wingsCC);
	wingsCC.editorIsCollapsed = true;
	for (int i = 0; i < 4; i++)
	{
		BoolParameter* p = wingsCC.addBoolParameter("Wing " + String(i), "If this is not checked, there is a problem either on this motor or wing", false);
		p->setControllableFeedbackOnly(true);
	}


	addChildControllableContainer(&controlsCC);
	takeOffTrigger = controlsCC.addTrigger("Take off", "Make the drone take off");
	rebootTrigger = controlsCC.addTrigger("Reboot", "Reboot the drone");
	stopTrigger = controlsCC.addTrigger("Stop", "Stop the drone");
	landTrigger = controlsCC.addTrigger("Land", "Land the drone");

	calibrateTrigger = controlsCC.addTrigger("Calibrate", "Reset the kalman filter");
	propCheckTrigger = controlsCC.addTrigger("Health Check", "Check propellers and motors.");

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
	positionDiff->setControllableFeedbackOnly(true);
	positionNoise = flightCC.addFloatParameter("Position Noise", "Overall position noise", 0, 0, 5);
	positionNoise->setControllableFeedbackOnly(true);

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
	for (auto& c : statusCC.controllables) c->setControllableFeedbackOnly(true);

	calibrationProgress = statusCC.addFloatParameter("Calibration Progress", "Progress of calibration", 0, 0, 1);
	analysisProgress = statusCC.addFloatParameter("Analysis Progress", "Progress of analysis", 0, 0, 1);

	memset(qualityPackets, 0, maxQualityPackets * sizeof(bool));

	//stateChanged(); //force powered_off take care

	setNiceName(droneId->stringValue());
	startThread();

}

CFDrone::~CFDrone()
{
	masterReference.clear();
}

void CFDrone::clearItem()
{
	signalThreadShouldExit();
	waitForThreadToExit(1000);

	if (cf != nullptr) cf->land(0, 4);
	color->setColor(Colours::black);

	batteryBlock.reset();
	posBlock.reset();
	calibBlock.reset();
}



void CFDrone::setupDrone()
{

	state->setValueWithData(DISCONNECTED);

	if (!enabled->boolValue()) return;

	try
	{
		String uri = getURI();

		DBG("Enter Lock here");
		//droneLock.enter();
		jassert(!(batteryBlock != nullptr && cf == nullptr));

		if (batteryBlock != nullptr) batteryBlock.reset();
		if (posBlock != nullptr) posBlock.reset();
		if (calibBlock != nullptr) calibBlock.reset();

		cf.reset(new Crazyflie(uri.toStdString(), EmptyLogger, consoleFunc));

		cf->setEmptyAckCallback(emptyAckFunc);
		cf->setLinkQualityCallback(linkQualityFunc);

		cf->requestParamToc();
		cf->requestLogToc();
		cf->requestMemoryToc();

		cf->logReset();

		batteryBlock.reset(new LogBlock<BatteryBlock>(cf.get(), { {"pm", "batteryLevel"}, { "pm","vbat" }, { "pm","state" } }, batteryBlockCallback));
		posBlock.reset(new LogBlock<PosBlock>(cf.get(), { {"stateEstimate","x"}, {"stateEstimate","y"}, {"stateEstimate","z"}, {"stabilizer","pitch"}, {"stabilizer","yaw"}, {"stabilizer","roll"} }, posBlockCallback));
		calibBlock.reset(new LogBlock<CalibBlock>(cf.get(), { {"kalman", "varPX"}, { "kalman","varPY" }, { "kalman","varPZ" } }, calibBlockCallback));


		batteryBlock->start(20);
		posBlock->start(5);


		NLOG(niceName, "Enable high level commander");
		setParam("commander", "enHighLevel", 1);

		Crazyflie::LighthouseBSGeometry bs1;
		Crazyflie::LighthouseBSGeometry bs2;
		cf->getLighthouseGeometries(bs1, bs2);

		Vector3D<float> bs1Pos(bs1.origin[0], bs1.origin[1], bs1.origin[2]);
		Vector3D<float> bs2Pos(bs2.origin[0], bs2.origin[1], bs2.origin[2]);

		NLOG(niceName, "Lighthouse GET geometries : " << bs1Pos.x << ", " << bs1Pos.y << ", " << bs1Pos.z << " / " << bs2Pos.x << ", " << bs2Pos.y << ", " << bs2Pos.z);

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
		
		
		cf->setLighthouseGeometries(bs1, bs2);


		cf->getLighthouseGeometries(bs1, bs2);
		bs1Pos = Vector3D<float>(bs1.origin[0], bs1.origin[1], bs1.origin[2]);
		bs2Pos = Vector3D<float>(bs2.origin[0], bs2.origin[1], bs2.origin[2]);

		NLOG(niceName, "Lighthouse after SET geometries : " << bs1Pos.x << ", " << bs1Pos.y << ", " << bs1Pos.z << " / " << bs2Pos.x << ", " << bs2Pos.y << ", " << bs2Pos.z);

		

		NLOG(niceName, "Drone is set up with URI : " << uri);
		DBG("Exit lock here");
		//droneLock.exit();
		state->setValueWithData(READY);

	}
	catch (std::exception e)
	{
		NLOGERROR(niceName, "Error during drone setup " << e.what());
		state->setValueWithData(DISCONNECTED);
	}

}


void CFDrone::setParam(String group, String name, var value)
{
	if (cf == nullptr)
	{
		DBG("Not connected, can't set param");
		return;
	}

	const Crazyflie::ParamTocEntry* pEntry = cf->getParamTocEntry(group.toStdString(), name.toStdString());
	if (pEntry == nullptr)
	{
		LOGWARNING("Parameter " << group << "." << name << " does not exist on this drone");
		return;
	}

	try
	{
		var newVal;
		switch (pEntry->type)
		{
		case Crazyflie::ParamType::ParamTypeInt8:
		case Crazyflie::ParamType::ParamTypeInt16:
		case Crazyflie::ParamType::ParamTypeInt32:
		case Crazyflie::ParamType::ParamTypeUint8:
		case Crazyflie::ParamType::ParamTypeUint16:
		case Crazyflie::ParamType::ParamTypeUint32:
			cf->setParam(pEntry->id, (int)value);
			newVal = cf->getParam<int>(pEntry->id);
			break;
		case Crazyflie::ParamType::ParamTypeFloat:
			cf->setParam(pEntry->id, (float)value);
			newVal = cf->getParam<float>(pEntry->id);
			break;
		}

		//LOG("Set param " << group << "." << name << " : " << value.toString() << ", newParam is " << newVal.toString());
		//LOG("Set param " << group << "." << name << " : " << value.toString() << ", newParam is " << newVal.toString());
	}
	catch (std::exception e)
	{
		NLOGERROR(niceName, "Error setting param " << group << "." << name << " : " << e.what());
	}
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
	calibrateTrigger->setEnabled(s == READY || s == WARNING);
	takeOffTrigger->setEnabled(s == READY);
	propCheckTrigger->setEnabled(s == READY || s == WARNING);
	landTrigger->setEnabled(s == TAKING_OFF || s == FLYING);
	rebootTrigger->setEnabled(s != TAKING_OFF && s != FLYING && s != LANDING);
	stopTrigger->setEnabled(s == READY || s == TAKING_OFF || s == FLYING || s == LANDING);
}

void CFDrone::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == enabled)
	{
		DroneState s = state->getValueDataAsEnum<DroneState>();
		state->setValueWithData(DISCONNECTED);
		if (s == DISCONNECTED || !enabled->boolValue()) stateChanged(); //force when not enabled or was already in this state to stop timers
	}
	else if (p == droneId)
	{
		setNiceName(droneId->stringValue());
		state->setValueWithData(DISCONNECTED);
	}
}

void CFDrone::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{

	DroneState currentState = state->getValueDataAsEnum<DroneState>();


	if (!enabled->boolValue()) return;

	if (c == state) stateChanged();
	else if (c == calibrateTrigger)
	{
		timeAtStartCalib = Time::getMillisecondCounter() / 1000.0; //to be sure
		state->setValueWithData(CALIBRATING);
	}
	else if (c == rebootTrigger)
	{
		state->setValueWithData(DISCONNECTED);
		//to port
		cf->reboot();
	}
	else if (c == stopTrigger)
	{
		//to port
		cf->stop();
		if (currentState == FLYING || currentState == TAKING_OFF || currentState == LANDING) state->setValueWithData(READY);
	}
	else if (c == takeOffTrigger)
	{
		if (currentState == READY && !lowBattery->boolValue() && !upsideDown->boolValue())
		{
			timeAtStartTakeOff = Time::getMillisecondCounter() / 1000.0;
			state->setValueWithData(TAKING_OFF);
		}
	}
	else if (c == landTrigger)
	{
		timeAtStartLanding = Time::getMillisecondCounter() / 1000.0;
		landingTime = jlimit<float>(2, 10, CFSettings::toLMFVector(realPosition->getVector()).y * 2);
		if (currentState == FLYING || currentState == TAKING_OFF) state->setValueWithData(LANDING);
	}
	else if (c == propCheckTrigger)
	{
		timeAtStartHealthCheck = Time::getMillisecondCounter() / 1000.0;
		state->setValueWithData(HEALTH_CHECK);
	}
	else if (c == realPosition)
	{
		double time = Time::getMillisecondCounterHiRes() / 1000.0;
		double deltaTime = time - lastRealPosTime;

		positionDiff->setVector((realPosition->getVector() - lastRealPos) / deltaTime);
		positionNoise->setValue(positionDiff->getVector().length());

		lastRealPosTime = time;
		lastRealPos = realPosition->getVector();
	}
	else if (c == lowBattery)
	{
		if (lowBattery->boolValue())
		{
			lightMode->setValueWithData(LightMode::FADE_COLOR);
			color->setColor(Colours::red);
			landTrigger->trigger();
		}
	}
	else if (c == charging)
	{
		lowBattery->setValue(false);
		if (charging->boolValue()) color->setColor(Colour(20,20,0));
		else color->setColor(Colours::black);
	}
	else if (c == upsideDown)
	{

	}
	else if (!lowBattery->boolValue())
	{
		if (c == color) setParam("ring", "fadeColor", (int64)color->getColor().getARGB());
		else if (c == headlight) setParam("ring", "headlightEnable", headlight->boolValue());
		else if (c == lightMode) setParam("ring", "effect", lightMode->getValueData());
		else if (c == fadeTime) setParam("ring", "fadeTime", fadeTime->floatValue());
		else if (c == stealthMode) setParam("platform", "stealthMode", stealthMode->boolValue());
	}
}


void CFDrone::consolePacketReceived(const char* msg)
{
	consoleBuffer += msg;
	if (String(msg).containsChar('\n'))
	{
		String lc = consoleBuffer.toLowerCase();
		if (lc.contains("fail")) NLOGERROR(niceName, consoleBuffer);
		else if (lc.contains("warning")) NLOGWARNING(niceName, consoleBuffer);
		else NLOG(niceName, consoleBuffer); consoleBuffer.clear();
	}
}

void CFDrone::rssiAckReceived(const crtpPlatformRSSIAck* ack)
{
	//Received
}

void CFDrone::linkQualityReceived(float quality)
{
	//DBG("Link quality :" << quality);
	linkQuality->setValue(quality);
}


void CFDrone::batteryBlockReceived(uint32, BatteryBlock* bLog)
{
	if (CFSettings::getInstanceWithoutCreating() == nullptr) return;

	batteryLevel->setValue(bLog->battery / 100.0f);
	voltage->setValue(bLog->voltage);

	if (voltage->floatValue() < CFSettings::getInstance()->minBattery->floatValue())
	{
		/*
		if (!lowBattery->boolValue() && !MultiTimer::isTimerRunning(LOWBAT_ID))
		{
			MultiTimer::startTimer(LOWBAT_ID, 1000.0f * CFSettings::getInstance()->lowBatteryTime->floatValue());
			NLOG(niceName, "Entering Low battery check");
		}
		*/
	}
	else
	{
		//stopTimer(LOWBAT_ID);
	}

	charging->setValue(bLog->charging > 0);
}

void CFDrone::posBlockReceived(uint32, PosBlock* pLog)
{
	if (Engine::mainEngine == nullptr || Engine::mainEngine->isClearing) return;

	realPosition->setVector(CFSettings::toLMFVector(Vector3D<float>(pLog->x, pLog->y, pLog->z)));
	orientation->setVector(pLog->rx, pLog->ry, pLog->rz);
	upsideDown->setValue(pLog->rz < -120 || pLog->rz > 120);
}

void CFDrone::calibBlockReceived(uint32, CalibBlock* cLog)
{

	//LOG("Variance " << cLog->varianceX << ", " << cLog->varianceY << ", " << cLog->varianceZ);
	double t = Time::getMillisecondCounter() / 1000.0;
	bool stab = cLog->varianceX < minConvergeDist && cLog->varianceY < minConvergeDist && cLog->varianceZ < minConvergeDist;
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
			calibBlock->stop();
			calibrationProgress->setValue(0);
			state->setValueWithData(READY);
		}
		else
		{
			calibrationProgress->setValue((float)((t - timeAtStartConverge) * 1.0f / minConvergeTime));
		}
	}
	else
	{
		timeAtStartConverge = 0;
		isCalibrated = false;
		calibrationProgress->setValue(0);
	}
}


void CFDrone::stateChanged()
{
	DroneState s = state->getValueDataAsEnum<DroneState>();

	updateControls();

	if (!enabled->boolValue()) return;

	switch (s)
	{
	case DISCONNECTED:
	{
	}
	break;

	case CALIBRATING:
	{
		isCalibrated = false;

		setParam("kalman", "resetEstimation", 1);
		setParam("kalman", "resetEstimation", 0);
		timeAtStartCalib = Time::getMillisecondCounter() / 1000.0;
		timeAtStartConverge = 0;
		lightMode->setValueWithData(DOUBLE_SPINNER);
		calibBlock->start(5);
	}
	break;

	case HEALTH_CHECK:
	{
		setParam("health", "startPropTest", 1);
		setParam("health", "startPropTest", 0);
		lightMode->setValueWithData(BATTERY_STATUS);
		timeAtStartHealthCheck = Time::getMillisecondCounter() / 1000.0;
	}
	break;

	case READY:
	{
		calibBlock->stop();

		lightMode->setValueWithData(FADE_COLOR);
		fadeTime->setValue(fadeTime->floatValue(), false, true);
		color->setColor(Colours::black);
	}
	break;

	case TAKING_OFF:
	{
		float takeOffTime = CFSettings::getInstance()->takeOffTime->floatValue();
		cf->takeoff(CFSettings::getInstance()->takeOffHeight->floatValue(), takeOffTime);
		timeAtStartTakeOff = Time::getMillisecondCounter() / 1000.0;
	}
	break;

	case FLYING:
	{
		targetPosition->setVector(realPosition->getVector());
		desiredPosition->setVector(realPosition->getVector());
	}
	break;

	case LANDING:
	{
		timeAtStartLanding = Time::getMillisecondCounter() / 1000.0;
		float h = CFSettings::toLMFVector(realPosition->getVector()).y;
		landingTime = jlimit<float>(2, 4, h*2);
		cf->land(h, landingTime);
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


void CFDrone::run()
{
	while (!threadShouldExit() && enabled->boolValue())
	{
		try
		{
			DroneState s = state->getValueDataAsEnum<DroneState>();

			switch (s)
			{
			case DISCONNECTED:
				setupDrone();
				break;


			case CALIBRATING:
				runCalibrate();
				break;

			case READY:
				targetPosition->setVector(realPosition->getVector());
				desiredPosition->setVector(realPosition->getVector());
				break;

			case TAKING_OFF:
				if (Time::getMillisecondCounter() / 1000.0 - timeAtStartTakeOff > CFSettings::getInstance()->takeOffTime->floatValue()) state->setValueWithData(FLYING);
				break;


			case FLYING:
				runFlying();
				break;

			case LANDING:
			{
				if (Time::getMillisecondCounter() / 1000.0 - timeAtStartLanding > landingTime)
				{
					//cf->stop();
					state->setValueWithData(READY);
				}
			}
			break;


			case HEALTH_CHECK:
				runHealthCheck();
				break;

			default:
				break;

			}

			if (s != DISCONNECTED)
			{
				cf->sendPing();
			}

		}
		catch (std::exception e)
		{
			NLOGERROR(niceName, "Error : " << e.what());
			state->setValue(DISCONNECTED);
		}

		sleep(20);
	}
}

void CFDrone::runConnect()
{
	cf->sendPing();
}

void CFDrone::runCalibrate()
{
	if (Time::getMillisecondCounter() / 1000.0 - timeAtStartCalib > calibTimeout)
	{
		isCalibrated = false;
		state->setValueWithData(WARNING);
	}
}

void CFDrone::runHealthCheck()
{
	if (Time::getMillisecondCounter() / 1000.0 - timeAtStartHealthCheck > 7)
	{
		timeAtStartCalib = Time::getMillisecondCounter() / 1000.0;
		state->setValueWithData(CALIBRATING);
	}
}

void CFDrone::runFlying()
{

	double deltaTime = Time::getMillisecondCounter() / 1000.0 - lastPhysicsUpdateTime;

	PhysicsCC::PhysicalState currentState = PhysicsCC::PhysicalState(targetPosition->getVector(), targetSpeed->getVector(), targetAcceleration->getVector());
	PhysicsCC::PhysicalState desiredState = PhysicsCC::PhysicalState(desiredPosition->getVector(), desiredSpeed->getVector(), desiredAcceleration->getVector());
	PhysicsCC::PhysicalState targetState = CFSettings::getInstance()->physicsCC.processPhysics(deltaTime, currentState, desiredState);
	targetPosition->setVector(targetState.position);
	targetSpeed->setVector(targetState.speed);
	targetAcceleration->setVector(targetState.acceleration);

	//Invert Z and Y depending on zIsVertical option in Project Settings (if zIsVertical is not checked, then don't swap y and z because crazyflie consider z as vertical)

	Vector3D<float> targetPos = CFSettings::toDroneVector(targetState.position);
	cf->sendPositionSetpoint(targetPos.x, targetPos.y, targetPos.z, CFSettings::getInstance()->disableYawCommand->boolValue() ? 0 : targetYaw->floatValue());

	//addCommand(CFCommand::createPosition(this, targetPos, CFSettings::getInstance()->disableYawCommand->boolValue()?0:targetYaw->floatValue()));
	lastPhysicsUpdateTime = Time::getMillisecondCounter() / 1000.0;
}



String CFDrone::getURI() const
{
	int targetRadio = floor(droneId->intValue() / 10); //10 drones / radio
	int targetChannel = droneId->intValue() * 2;
	String droneAddress = "E7E7E7E7" + String::formatted("%02d", droneId->intValue());
	return "radio://" + String(targetRadio) + "/" + String(targetChannel) + "/2M/" + droneAddress;
}


/* ----------------- LEGACY ----------------- */
/*
void CFDrone::addTakeoffCommand()
{
	float t = (Time::getMillisecondCounter() - timeAtStartTakeOff) / 1000.0f;
	float relTime = t / CFSettings::getInstance()->takeOffTime->floatValue();

	bool heightIsGood = Time::getMillisecondCounter() > timeAtStartLanding + 1 && realPosition->y >= CFSettings::getInstance()->takeOffHeight->floatValue();

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

		cf->sendSetpoint(0, 0, 0, thrust * 10000);

		//addCommand(CFCommand::createSetPoint(this, 0, 0, 0, thrust * 10000));
		LOG("add thrust takeoff command with thrust " << thrust);
	} else
	{
		float velZ = CFSettings::getInstance()->takeOffCurve.getValueForPosition(relTime) * CFSettings::getInstance()->takeOffMaxSpeed->floatValue();
		LOG("add velocity takeoff command with force " << velZ);
		//cf->send(0, 0, 0, thrust * 10000);
		//addCommand(CFCommand::createVelocity(this, Vector3D<float>(0, 0, velZ), 0));
	}

}
*/