/*
  ==============================================================================

	Drone.cpp
	Created: 12 Jun 2018 4:34:07pm
	Author:  Ben

  ==============================================================================
*/

#include "Drone.h"
#include "CFSettings.h"


Drone::Drone() :
	BaseItem("Drone"),
	Thread("DroneThread"),
	cf(nullptr),
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
	noPongCount(0),
	lastTime(0)
{
	addChildControllableContainer(&radioCC);
	autoRadio = radioCC.addBoolParameter("Auto radio", "If enabled, radio will automatically be set depending on the channel : Channel 0-> 9 = radio 0, Channel 10->19 = radio 1...", false);
	autoChannel = radioCC.addBoolParameter("Auto channel", "If enabled, the last byte of the address will be used to automatically set the drone channel : 00->0F = Channel 0, 10->1F = Channel 10, 20->2F = Channel 20...", false);
	targetRadio = radioCC.addIntParameter("Radio", "The USB Crazyradio to use to connect to the drone", 0, 0, 16);
	channel = radioCC.addIntParameter("Channel", "The Channel set up in the drone", 80, 0, 125);
	baudRate = radioCC.addEnumParameter("Baud Rate", "The baudrate to use to connect to the drone");
	baudRate->addOption("2M", Crazyradio::Datarate_2MPS)->addOption("250K", Crazyradio::Datarate_250KPS)->addOption("1M", Crazyradio::Datarate_1MPS);
	address = radioCC.addStringParameter("Address", "Address of the drone, without the 0x", "E7E7E7E7E7");

	addChildControllableContainer(&controlsCC);
	connectTrigger = controlsCC.addTrigger("Connect", "Connect to the drone");
	calibrateTrigger = controlsCC.addTrigger("Calibrate", "Reset the kalman filter");
	takeOffTrigger = controlsCC.addTrigger("Take off", "Make the drone take off");
	landTrigger = controlsCC.addTrigger("Land", "Land the drone");
	stopTrigger = controlsCC.addTrigger("Stop", "Stop the drone");
	rebootTrigger = controlsCC.addTrigger("Reboot", "Reboot the drone");

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
	lightMode->addOption("Off", 0)->addOption("White spinner", 1)->addOption("Color spinner", 2)->addOption("Tilt effect", 3) \
		->addOption("Brightness", 4)->addOption("Color spinner2", 5)->addOption("Double spinner", 6)->addOption("Solid color", 7) \
		->addOption("Factory test", 8)->addOption("Battery status", 9)->addOption("Boat lights", 10)->addOption("Alert", 11)->addOption("Gravity", 12)->addOption("Memory", 13)->addOption("Fade Color", 14);
	color = lightingCC.addColorParameter("Light Color", "The color of the led ring", Colours::black);
	fadeTime = lightingCC.addFloatParameter("Fade time", "The time to fade from one color to another", .5f, 0, 10);
	color->isSavable = false;
	headlight = lightingCC.addBoolParameter("Headlight", "Headlight", false);
	headlight->isSavable = false;
	stealthMode = lightingCC.addBoolParameter("Stealth Mode", "When in stealthMode, the system leds are off", false);



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

	selfTestProblem = statusCC.addBoolParameter("SelfTest Problem", "There was a problem during the selfTest", false);
	batteryProblem = statusCC.addBoolParameter("Battery Problem", "If on, you should replace this battery !", false);
	for (auto &c : statusCC.controllables) c->setControllableFeedbackOnly(true);

	calibrationProgress = statusCC.addFloatParameter("Calibration Progress", "Progress of calibration", 0, 0, 1);
	analysisProgress = statusCC.addFloatParameter("Analysis Progress", "Progress of analysis", 0, 0, 1);

	statusCC.addChildControllableContainer(&decksCC);
	decksCC.addBoolParameter("bcLedRing", "LED-Ring", false);
	decksCC.addBoolParameter("bcQi", "Qi charger", false);
	decksCC.addBoolParameter("bcBuzzer", "Buzzer", false);
	decksCC.addBoolParameter("bcBigQuad", "Big quad", false);
	decksCC.addBoolParameter("bcDWM", "UWB LPS", false);
	decksCC.addBoolParameter("bcUSD", "Micro - SD", false);
	decksCC.addBoolParameter("bcZRanger", "Z - Ranger", false);
	decksCC.addBoolParameter("bcFlow", "Flow", false);
	decksCC.addBoolParameter("bcOA", "Obstacle Avoidance", false);
	decksCC.addBoolParameter("bcMultiranger", "Multi - ranger", false);
	decksCC.addBoolParameter("bcMocap", "Mocap marker deck", false);
	decksCC.addBoolParameter("bcZRanger2", "Z - Ranger v2", false);
	decksCC.addBoolParameter("bcFlow2", "Flow V2", false);
	for (auto &c : decksCC.controllables) c->setControllableFeedbackOnly(true);

	statusCC.addChildControllableContainer(&wingsCC);
	for (int i = 0; i < 4; i++)
	{
		BoolParameter * p = wingsCC.addBoolParameter("Wing " + String(i), "If checked, there is a problem either on this motor or wing", false);
		p->setControllableFeedbackOnly(true);
	}


	startThread();
}
Drone::~Drone()
{
	signalThreadShouldExit();
	waitForThreadToExit(2000);
}

void Drone::onContainerParameterChangedInternal(Parameter * p)
{
	if (p == enabled)
	{
		if (enabled->boolValue())
		{
			startThread();
		} else
		{
			signalThreadShouldExit();
			waitForThreadToExit(1000);

			state->setValue(POWERED_OFF);
		}
	}
}

void Drone::onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c)
{
	if (c == state)
	{
		stateUpdated();
	}

	DroneState curState = state->getValueDataAsEnum<DroneState>();

	if (c == state)
	{
		connectTrigger->setEnabled(curState != POWERED_OFF);
		takeOffTrigger->setEnabled(curState == READY);
		landTrigger->setEnabled(curState == FLYING);
		calibrateTrigger->setEnabled(curState != DISCONNECTED && curState != POWERED_OFF);
		rebootTrigger->setEnabled(curState != DISCONNECTED && curState != POWERED_OFF);

	} else if (c == state || c == autoChannel || c == autoRadio)
	{
		targetRadio->setEnabled(!autoRadio->boolValue() && curState == POWERED_OFF || curState == DISCONNECTED);
		channel->setEnabled(!autoChannel->boolValue() && curState == POWERED_OFF || curState == DISCONNECTED);
		baudRate->setEnabled(curState == POWERED_OFF || curState == DISCONNECTED);
		address->setEnabled(curState == POWERED_OFF || curState == DISCONNECTED);
	}

	if (c == linkQuality)
	{
		if (linkQuality->floatValue() == 0 && curState != CONNECTING)
		{
			NLOG(niceName, "Lost connection to drone");
			state->setValueWithData(POWERED_OFF);
		}
	}

	if (c == address || c == autoRadio || c == autoChannel)
	{
		int64 lastByte = address->stringValue().getHexValue64() & 0xFF;
		int rIndex = floor(lastByte / 16);
		if (autoRadio->boolValue()) targetRadio->setValue(rIndex);
		if (autoChannel->boolValue()) channel->setValue(rIndex * 10);
	}

	if (!enabled->boolValue()) return;


	if (c == headlight) setParam("ring", "headlightEnable", headlight->boolValue());
	else if (c == lightMode) setParam("ring", "effect", (int)lightMode->getValueData());
	else if (c == color) setParam("ring", "fadeColor", color->getColor().getARGB());
	else if (c == fadeTime) setParam("ring", "fadeTime", fadeTime->floatValue());
	else if (c == stealthMode) setParam("platform", "stealthMode", stealthMode->intValue());


	if (cf == nullptr) return;

	if (c == address) cf->setRadioAddress(address->stringValue());
	else if (c == channel) cf->setRadioChannel(channel->intValue());
	else if (c == baudRate) cf->setRadioBaudrate(baudRate->getValueDataAsEnum<Crazyradio::Datarate>());

	else if (c == connectTrigger && curState != POWERED_OFF) state->setValueWithData(CONNECTING);
	else if (c == takeOffTrigger && curState == READY) state->setValueWithData(TAKING_OFF);
	else if (c == landTrigger && curState == FLYING) state->setValueWithData(LANDING);
	else if (c == rebootTrigger && curState != FLYING)
	{
		state->setValueWithData(POWERED_OFF);
		try
		{
			cf->reboot();
		} catch (std::runtime_error &)
		{
			DBG("Timeout on reboot !");
		}
	}

	if (curState == READY)
	{
		if (c == calibrateTrigger) state->setValueWithData(CALIBRATING);
		if (c == analyzeTrigger) state->setValueWithData(ANALYSIS);
	}

	if (curState == FLYING || curState == TAKING_OFF || curState == LANDING)
	{
		if (c == stopTrigger)
		{
			cf->sendStop();
			state->setValueWithData(READY);
		}
	}
}

void Drone::stateUpdated()
{
	DroneState s = state->getValueDataAsEnum<DroneState>();

	DBG("Status update : " << state->getValueKey());

	switch (s)
	{

	case POWERED_OFF:
		try
		{
			if (batteryLogBlock != nullptr) batteryLogBlock->stop();
			if (posLogBlock != nullptr) posLogBlock->stop();
		} catch (std::runtime_error &)
		{
			DBG("Timeout resetting log blocks");
		}

		break;

	case DISCONNECTED:
		if (CFSettings::getInstanceWithoutCreating() != nullptr && CFSettings::getInstance()->autoConnect->boolValue()) state->setValueWithData(CONNECTING);
		break;

	case CONNECTING:
		break;

	case CALIBRATING:
		lightMode->setValueWithKey("Double spinner");
		if (calibLog != nullptr) calibLog->stop();
		calibLog = nullptr;
		break;

	case ANALYSIS:
		lightMode->setValueWithKey("Battery status");
		break;

	case READY:
	{
		timeAtStartTakeOff = 0;
		timeAtStartLanding = 0;
		lightMode->setValueWithKey("Fade Color");
		color->setColor(Colours::black);

		desiredPosition->setVector(realPosition->getVector());
		targetPosition->setVector(realPosition->getVector());
	}
	break;

	case TAKING_OFF:
		break;

	case FLYING:
		timeAtStartTakeOff = 0;
		timeAtStartLanding = 0;
		break;

	case LANDING:
		break;

	case WARNING:
		lightMode->setValueWithKey("Boat lights");
		break;

	case ERROR:
		lightMode->setValueWithKey("Alert");
		break;

	}


}


void Drone::run()
{
	Random r;
	int numR = r.nextInt(Range<int>(10, 20));
	for (int i = 0; i < numR; i++)
	{
		if (threadShouldExit()) return;
		sleep(10);
	}

	while (!threadShouldExit() && cf == nullptr)
	{
		try
		{
			setupCF();
		} catch (std::runtime_error &e)
		{
			NLOG(niceName, "Runtime error " << e.what());
			cf = nullptr;
			for (int i = 0; i < 50; i++)
			{
				if (threadShouldExit()) return;
				sleep(10);
			}
		}
	}

	state->setValueWithData(POWERED_OFF);


	try
	{
		while (!threadShouldExit())
		{
			bool sendingData = false;

			DroneState s = state->getValueDataAsEnum<DroneState>();
			switch (s)
			{
			case POWERED_OFF:
				break;

			case DISCONNECTED:
				break;

			case CONNECTING:
				connect();
				break;

			case CALIBRATING:
				processCalibration();
				break;

			case ANALYSIS:
				break;

			case READY:
				break;

			case TAKING_OFF:
				updateTakeOff();
				sendingData = true;
				break;

			case FLYING:
				updateFlyingPosition();
				sendingData = true;
				break;

			case LANDING:
				updateLanding();
				sendingData = true;
				break;

			case WARNING:
				break;

			case ERROR:
				break;
			}

			if (!sendingData)
			{
				if (s == POWERED_OFF || s == DISCONNECTED) ping(); 
				else if(cf != nullptr) cf->sendPing();
			}

			sleep(r.nextInt(10));
		}

		//Exit
		if (batteryLogBlock != nullptr) batteryLogBlock->stop();
		if (posLogBlock != nullptr) posLogBlock->stop();
		batteryLogBlock = nullptr;
		posLogBlock = nullptr;
		calibLog = nullptr;
		cf = nullptr;

		state->setValueWithData(DISCONNECTED);
	} catch (std::runtime_error &e)
	{
		NLOG(niceName, "Runtime error " << e.what());
	} catch (std::exception &e)
	{
		NLOG(niceName, "Exception " << e.what());
	}

}

void Drone::setupCF()
{
	cf = nullptr; //delete previous
	cf = new Crazyflie(targetRadio->intValue(), channel->intValue(), baudRate->getValueDataAsEnum<Crazyradio::Datarate>(), address->stringValue());
	//Set the callbacks and logs
	std::function<void(const char *)> consoleF = std::bind(&Drone::consoleCallback, this, std::placeholders::_1);
	cf->setConsoleCallback(consoleF);

	std::function<void(const crtpPlatformRSSIAck *)> ackF = std::bind(&Drone::emptyAckCallback, this, std::placeholders::_1);
	cf->setEmptyAckCallback(ackF);

	std::function<void(float)> linkF = std::bind(&Drone::linkQualityCallback, this, std::placeholders::_1);
	cf->setLinkQualityCallback(linkF);

}

void Drone::ping()
{
	if (cf == nullptr) return;


	if (Time::getApproximateMillisecondCounter() > lastPingTime + pingTime)
	{
		lastPingTime = Time::getApproximateMillisecondCounter();

		bool result = cf->sendPing();

		DroneState s = state->getValueDataAsEnum<DroneState>();

		//NLOG(niceName, "Send ping " << (int)result);

		if (result) {
			noPongCount = 0;
			if (s == POWERED_OFF && result) state->setValueWithData(DISCONNECTED);
		} else
		{
			noPongCount++;
			if (noPongCount > maxNoPongCount) state->setValueWithData(POWERED_OFF);
		}

	}
}

void Drone::connect()
{
	NLOG(niceName, "Connecting...");

	//Reset all parameters

	lowBattery->setValue(false);
	batteryProblem->setValue(false);
	selfTestProblem->setValue(false);
	batteryLevel->setValue(0);
	charging->setValue(false);
	targetPosition->setVector(0, 0, 0);
	realPosition->setVector(0, 0, 0);
	yaw->setValue(0);
	linkQuality->setValue(0);
	lightMode->setValue("Off");
	color->setColor(Colours::black);
	headlight->setValue(false);

	Array<WeakReference<Parameter>> deckParams = decksCC.getAllParameters();
	for (auto & p : deckParams) p->setValue(false);
	Array<WeakReference<Parameter>> wingsParam = wingsCC.getAllParameters();
	for (auto & w : wingsParam) w->setValue(false);

	try
	{
		cf->requestParamToc();
		bool selfTest = cf->requestParamValue<uint8>(cf->getParamTocEntry("system", "selftestPassed")->id);

		int attempt = 0;
		while (!selfTest && attempt < 3)
		{
			for (int i = 0; i < 100; i++)
			{
				if (threadShouldExit()) return;
				ping();
				sleep(10);
			}
			
			DBG("Self test failed, maybe it's booting ? attempt " << attempt << " of 6");
			selfTest = cf->requestParamValue<uint8>(cf->getParamTocEntry("system", "selftestPassed")->id);
			attempt++;
		}

		selfTestProblem->setValue(!selfTest);

		if (!selfTest)
		{
			NLOGERROR(niceName, "Self test failed");
			state->setValueWithData(WARNING);
			return;
		}

		if (batteryLogBlock != nullptr) batteryLogBlock->stop();
		if (posLogBlock != nullptr) posLogBlock->stop();
		batteryLogBlock = nullptr;
		posLogBlock = nullptr;

		cf->logReset();
		cf->requestLogToc();

		if (droneHasLogVariable("pm", "batteryLevel") && /*droneHasLogVariable("pm", "lowBattery") &&*/ droneHasLogVariable("pm", "state"))
		{
			std::function<void(uint32_t, BatteryLog *)> cb = std::bind(&Drone::batteryLogCallback, this, std::placeholders::_1, std::placeholders::_2);
			batteryLogBlock = new LogBlock<BatteryLog>(cf, { { "pm","batteryLevel" }/*,{ "pm","lowBattery" }*/,{ "pm","state" } }, cb);
			batteryLogBlock->start(50);
		} else
		{
			NLOGWARNING(niceName, "Drone doesn't have batteryLevel log");
		}


		std::function<void(uint32_t, PosLog *)> fcb = std::bind(&Drone::posLogCallback, this, std::placeholders::_1, std::placeholders::_2);
		posLogBlock = new LogBlock<PosLog>(cf, { { "kalman","stateX" },{ "kalman","stateY" },{ "kalman","stateZ" } }, fcb);
		posLogBlock->start(5); //5 = 20fps

		NLOG(niceName, "Connected");

		if (CFSettings::getInstance()->analyzeAfterConnect->boolValue()) state->setValueWithData(ANALYSIS);
		else if (CFSettings::getInstance()->calibAfterConnect->boolValue()) state->setValueWithData(CALIBRATING);
		else state->setValueWithData(READY);

	} catch (std::runtime_error &e)
	{
		NLOGERROR(niceName, "Error on drone setup : " << e.what());
	} catch (std::exception &e)
	{
		NLOGERROR(niceName, "Exception on drone setup : " << e.what());
	}
}

void Drone::calibrate()
{

}

void Drone::processCalibration()
{
	if (cf == nullptr) return;
	if (calibLog != nullptr) return;

	timeAtStartConverge = 0;
	calibrationProgress->setValue(0);

	std::function<void(uint32_t, CalibLog *)> fcb = std::bind(&Drone::calibLogCallback, this, std::placeholders::_1, std::placeholders::_2);
	calibLog = new LogBlock<CalibLog>(cf, { { "kalman","varPX" },{ "kalman","varPY" },{ "kalman","varPZ" } }, fcb);
	calibLog->start(2);

	setParam("kalman", "resetEstimation", 1);
	sleep(50);
	setParam("kalman", "resetEstimation", 0);
}

void Drone::takeOff()
{
	if (cf == nullptr) return;
}

void Drone::updateTakeOff()
{
	if (cf == nullptr) return;
	if (CFSettings::getInstanceWithoutCreating() == nullptr) return;

	if (timeAtStartTakeOff == 0) timeAtStartTakeOff = Time::getMillisecondCounter() / 1000.0f;

	float t = Time::getMillisecondCounter() / 1000.0f;
	float toTime = CFSettings::getInstance()->takeOffTime->floatValue();
	float relTime = (t - timeAtStartTakeOff) / toTime;
	float p = CFSettings::getInstance()->takeOffCurve.getValueForPosition(relTime) * CFSettings::getInstance()->takeOffMaxSpeed->floatValue();

	desiredPosition->setVector(realPosition->getVector());
	targetPosition->setVector(realPosition->getVector());
	lastTargetPosition = targetPosition->getVector();

	cf->sendVelocitySetpoint(0, 0, p, 0);

	if (relTime >= 1)
	{
		lastTargetPosition = targetPosition->getVector();
		lastTime = 0;
		state->setValueWithData(FLYING);
	}
}

void Drone::updateFlyingPosition()
{
	if (cf == nullptr || !enabled->boolValue()) return;
	if (CFSettings::getInstanceWithoutCreating() == nullptr) return;


	Vector3D<float> tPos = desiredPosition->getVector();

	if (CFSettings::getInstance()->physicsCC.enabled->boolValue())
	{
		double t = Time::getMillisecondCounterHiRes() / 1000.0f;

		if (lastTime == 0)
		{
			lastSpeed = Vector3D<float>();
			lastTargetPosition = Vector3D<float>(tPos.x, tPos.y, tPos.z);
		}

		if (lastTime > 0)
		{
			deltaTime = t - lastTime;

			//targetSpeed = tPos - lastTargetPosition;
			//targetAcceleration = targetSpeed - lastSpeed;

			float force = CFSettings::getInstance()->physicsCC.forceFactor->floatValue();
			float frot = CFSettings::getInstance()->physicsCC.frotFactor->floatValue();

			//Spring
			Vector3D<float> acc = (tPos - lastTargetPosition) * force;

			//General physics
			acc -= Vector3D<float>(lastSpeed.x*frot, lastSpeed.y*frot, lastSpeed.z*frot); //frottement - general

			Vector3D<float> tSpeed = lastSpeed + acc * deltaTime; // speed calculation - general
			targetPosition->setVector(lastTargetPosition + tSpeed * deltaTime); // pos calculation - general
			targetSpeed->setVector(tSpeed);
			targetAcceleration->setVector(acc);

			lastTargetPosition = targetPosition->getVector();
			lastSpeed = Vector3D<float>(tSpeed.x, tSpeed.y, tSpeed.z);
		}

		lastTime = t;

		//DBG("Tpos : " << tPos.x << ", " << tPos.y << ", " << tPos.z);
	} else
	{
		targetPosition->setVector(tPos);
	}

	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	cf->sendPositionSetpoint(targetPosition->x, zIsVertical?targetPosition->y:targetPosition->z, zIsVertical?targetPosition->z:targetPosition->y, yaw->floatValue());
}

void Drone::land()
{

}

void Drone::updateLanding()
{
	if (timeAtStartLanding == 0)
	{
		float h = CFSettings::getInstance()->zIsVertical->boolValue() ? realPosition->z : realPosition->y;
		landingTime = jmax<float>(h * 2, 1);
		timeAtStartLanding = Time::getMillisecondCounter() / 1000.0f;
	}

	float t = Time::getMillisecondCounter() / 1000.0f;
	float toTime = CFSettings::getInstance()->takeOffTime->floatValue();
	cf->sendVelocitySetpoint(0, 0, -.5f, 0);
	float relTime = (t - timeAtStartLanding) / toTime;
	if (relTime >= 1) state->setValueWithData(READY);
}

template<class T>
bool Drone::setParam(String group, String paramID, T value, bool force)
{
	if (cf == nullptr) return false;
	DroneState s = state->getValueDataAsEnum<DroneState>();
	if (!force && (s == POWERED_OFF || s == DISCONNECTED || !enabled->boolValue())) return false;

	try
	{
		SpinLock::ScopedLockType lock(paramLock);

		const Crazyflie::ParamTocEntry * pte = cf->getParamTocEntry(group.toStdString(), paramID.toStdString());
		if (pte == nullptr)
		{
			NLOGWARNING(niceName, "Param " << group << "." << paramID << " doesn't exist");
			return false;
		}

		NLOG(niceName, "Set param : " << group << "." << paramID << " : " << (int)value);
		cf->setParam(pte->id, value);

	} catch (std::runtime_error &e)
	{
		NLOGERROR(niceName, "Error setting param " << group << "." << paramID << " : " << e.what());
		return false;
	}

	return true;
}


void Drone::emptyAckCallback(const crtpPlatformRSSIAck * a)
{
	//ack
	//DBG("Empty ack");
	//if(state->getValueDataAsEnum<DroneState>() == POWERED_OFF) state->setValue(DISCONNECTED);
}

void Drone::linkQualityCallback(float val)
{
	linkQuality->setValue(val);
	//DBG(niceName + " Link quality callback : " + String(val));
}

void Drone::batteryLogCallback(uint32_t, BatteryLog * data)
{
	batteryLevel->setValue(data->battery);
	//lowBattery->setValue(data->lowBattery);
	charging->setValue(data->charging > 0);
}

void Drone::posLogCallback(uint32_t, PosLog * data)
{
	if (CFSettings::getInstanceWithoutCreating() == nullptr) return;
	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	realPosition->setVector(data->x, zIsVertical ? data->y : data->z, zIsVertical ? data->z : data->y);
}

void Drone::calibLogCallback(uint32_t, CalibLog * data)
{
	//DBG("Variance " << data->varianceX << ", " << data->varianceY << ", " << data->varianceZ);
	uint64 t = Time::currentTimeMillis();
	bool stab = /*data->varianceX > 0 && data->varianceY > 0 && data->varianceZ > 0 &&*/ data->varianceX < minConvergeDist && data->varianceY < minConvergeDist && data->varianceZ < minConvergeDist;
	if (stab)
	{
		if (timeAtStartConverge == 0) timeAtStartConverge = t;
		if (t > timeAtStartConverge + minConvergeTime)
		{
			NLOG(niceName, "Calibrated");
			calibLog->stop();
			calibLog = nullptr;
			state->setValueWithData(READY);
		} else
		{
			calibrationProgress->setValue((float)((t - timeAtStartConverge)*1.0f / minConvergeTime));
		}
	} else
	{
		timeAtStartConverge = 0;
		calibrationProgress->setValue(0);
	}
}

bool Drone::allDecksAreConnected()
{
	Array<WeakReference<Parameter>> deckParams = decksCC.getAllParameters();
	int numDecks = 0;
	for (auto &p : deckParams)
	{
		if (p->boolValue()) numDecks++;
	}

	//hardcoded
	return numDecks == 2;
}

bool Drone::droneHasLogVariable(String group, String name)
{
	bool found = false;
	std::for_each(cf->logVariablesBegin(), cf->logVariablesEnd(),
		[&](const Crazyflie::LogTocEntry& entry) {
		if (group == String(entry.group) && name == String(entry.name))
		{
			found = true;
		}
	}
	);

	return found;
}



var Drone::getJSONData()
{
	var data = BaseItem::getJSONData();
	data.getDynamicObject()->setProperty("radio", radioCC.getJSONData());
	return data;
}

void Drone::loadJSONDataInternal(var data)
{
	BaseItem::loadJSONDataInternal(data);
	radioCC.loadJSONData(data.getProperty("radio", var()));

	if (enabled->boolValue())
	{
		signalThreadShouldExit();
		waitForThreadToExit(1000);
		startThread();

	}

}



void Drone::consoleCallback(const char * c)
{
	consoleBuffer += String(c);

	if (consoleBuffer.endsWith("\n"))
	{
		if (consoleBuffer.toLowerCase().contains("error") || consoleBuffer.toLowerCase().contains("fail")) NLOGERROR(address->stringValue(), consoleBuffer);
		else NLOG(address->stringValue(), consoleBuffer);

		consoleBuffer.clear();
	}
}

