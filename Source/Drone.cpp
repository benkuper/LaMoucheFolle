/*
  ==============================================================================

	Drone.cpp
	Created: 19 Oct 2017 7:33:14pm
	Author:  Ben

  ==============================================================================
*/

#include "Drone.h"
#include "NodeManager.h"
#include "DroneManager.h"

Drone::Drone() :
    BaseItem("Drone"),
    Thread("DroneInitThread"),//2s no packet = drone disconnected
	cf(nullptr),
	upsideDownFrozen(false),
	timeAtBelowLowBattery(0),
	ackTimeout(2000), //2s no packet = drone disconnected,
	timeAtLaunch(0)
{
	targetRadio = addIntParameter("Radio", "Target Radio to connect", 0, 0, 16);
	channel = addIntParameter("Channel", "Target channel of the drone", 40, 0, 200);
	speed = addEnumParameter("Baudrate", "Speed of the connection");
	speed->addOption("2M", Crazyradio::Datarate_2MPS)->addOption("250K", Crazyradio::Datarate_250KPS)->addOption("1M", Crazyradio::Datarate_1MPS);

	address = addStringParameter("Address", "Address of the drone, without the 0x", "E7E7E7E7E7");

	targetPosition = addPoint3DParameter("Target", "Target Position for the drone");
	targetPosition->setBounds(-10,0,-10, 10, 10, 10);
	targetPosition->isSavable = false;

	realPosition = addPoint3DParameter("Real Position", "Position feedback from the drone");
	realPosition->setBounds(-10,0,-10, 10, 10, 10);
	realPosition->isSavable = false;
	realPosition->isControllableFeedbackOnly = true;

	orientation = addPoint3DParameter("Orientation", "Target Position for the drone");
	orientation->setBounds(-180,-180,-180, 180,180,180);
	orientation->isSavable = false;
	orientation->isControllableFeedbackOnly = true;

	absoluteMode = addBoolParameter("Absolute Mode", "Absolute positionning", false);
	isFlying = addBoolParameter("Is Flying", "Is the drone flying ?",false);
	isFlying->isControllableFeedbackOnly = true;

	yaw = addFloatParameter("Yaw", "Horizontal rotation of the drone", 0, 0, 360);
	yaw->isSavable = false;

	droneState = addEnumParameter("State", "Drone State");
	droneState->addOption("Disconnected", DISCONNECTED)->addOption("Connecting", CONNECTING)->addOption("Stabilizing",STABILIZING)->addOption("Ready", READY)->addOption("Error", ERROR);
	droneState->isEditable = false;
	droneState->isSavable = false;
	droneState->isSavable = false;

	connectTrigger = addTrigger("Connect", "Init the connection to the radio");

	logParamsTOC = addTrigger("Get Params TOC", "Log all params from the drone");
	logLogsTOC = addTrigger("Get Log TOC", "Log logs entries from the drone");

	taskDump = addTrigger("TaskDump", "TD");
	writeToAnchors = addTrigger("Write position to nodes","Write the positions from the node manager to anchors");

	/*
	initPIDSettings = addBoolParameter("Init PID Settings", "Include PID settings in init", false);

	kd = addPoint3DParameter("Kd", "Kd settings for the PID tuning (drone reference, z is vertical axis)");
	ki = addPoint3DParameter("Ki", "Ki settings for the PID tuning (drone reference, z is vertical axis)");
	kp = addPoint3DParameter("Kp", "Kp settings for the PID tuning (drone reference, z is vertical axis)");
	
	kd->setBounds(0, 0, 0, 5, 5, 5); 
	kd->setVector(0, 0, 0);
	kd->defaultValue = kd->value;

	ki->setBounds(0, 0, 0, 5, 5, 5); 
	ki->setVector(0, 0, .5f);
	ki->defaultValue = ki->value;

	kp->setBounds(0, 0, 0, 5,5,5);
	kp->setVector(2, 2, 2);
	kp->defaultValue = kp->defaultValue;

	xyVelMax = addFloatParameter("Z Vel Max", "For the PID tuning", 1, .1f, 3);
	zVelMax = addFloatParameter("XY Vel Max", "For the PID tuning", 1, .1f, 3);
	rpLimit = addFloatParameter("RP Limit", "For the PID tuning (RollPitch limit ?)", 20, 5, 50);
	*/

	initAnchorPos = addBoolParameter("Init Anchor Pos", "Include anchor pos in init", false);

	launchTrigger = addTrigger("Launch", "Launch");
	stopTrigger = addTrigger("Stop", "Stop");
	syncTrigger = addTrigger("Sync Position", "Sync position");

	launchingMode = addBoolParameter("Is Launching", "Is the drone in launching phase", false);

	resetKalmanTrigger = addTrigger("Reset Kalman Estimation", "Reset Kalman Filter Estimation");

	lightMode = addEnumParameter("LightMode", "Led Preset");
	lightMode->addOption("Off", 0)->addOption("White spinner", 1)->addOption("Color spinner",2)->addOption("Tilt effect", 3) \
		->addOption("Brightness", 4)->addOption("Color spinner2", 5)->addOption("Double spinner", 6)->addOption("Solid color", 7) \
		->addOption("Factory test", 8)->addOption("Battery status", 9)->addOption("Boat lights", 10)->addOption("Alert", 11)->addOption("Gravity", 12);
	lightMode->isSavable = false;

	color = new ColorParameter("Light Color", "LightColor", Colours::black);
	addParameter(color);

	headlight = addBoolParameter("Headlight", "Headlight", false);

	autoReconnect = addBoolParameter("Auto Reconnect", "Auto Reboot drone if connected but in bad state", false);
	autoKillUpsideDown = addBoolParameter("Auto Kill Upside-Down", "If drone is upside down, will set it in error state", false);
	freezeOnUpsideDown = addBoolParameter("Freeze Upside-Down", "If drone is upside down and autokill set to false, will not send commands to drone when upside down", true);

	linkQuality = addFloatParameter("Link Quality", "Quality of radio communication with the drone", 0, 0, 1);
	linkQuality->isEditable = false;
	linkQuality->isSavable = false;

	voltage = addFloatParameter("Voltage", "Voltage of the drone. /!\\ When flying, there is a massive voltage drop !", 0, 0, 4.4f);
	voltage->setRange(DroneManager::getInstance()->flyingLowBatteryThreshold->floatValue(), voltage->maximumValue); 
	voltage->isEditable = false;
	voltage->isSavable = false;
	

	charging = addBoolParameter("Is Charging", "Is the drone connected to a power source", false);
	charging->isEditable = false;
	charging->isSavable = false;

	lowBattery = addBoolParameter("Low Battery", "Low battery (measured when not flying)", false);
	lowBattery->isEditable = false;
	lowBattery->isSavable = false;

	inTrigger = addTrigger("Activity IN", "If received any communication from the drone");
	outTrigger = addTrigger("Activity OUT", "If any data has been sent to the drone");
	inTrigger->hideInEditor = true;
	inTrigger->isEditable = false;
	outTrigger->isEditable = false;
	outTrigger->hideInEditor = true;

	enableLogConsole = addBoolParameter("Enable log console", "", false);
	enableLogParams = addBoolParameter("Enable log Params", "", false);

}

Drone::~Drone()
{
	stopCFThread();
}

void Drone::launchCFThread()
{
	stopCFThread();
	startThread();
}

void Drone::stopCFThread()
{
	droneState->setValueWithData(DISCONNECTED);

	if (!isThreadRunning()) return;
	signalThreadShouldExit();
	waitForThreadToExit(1000);
}



void Drone::onContainerParameterChangedAsync(Parameter * p, const var &)
{
	if (p == orientation)
	{
        Vector3D<float> ov = orientation->getVector();
		bool isUpsideDown = (ov.z > 160 || ov.z < -160) && fabsf(ov.x) < 20;
		bool upsideDownAndShouldDoSomething = autoKillUpsideDown->boolValue()
			&& (droneState->getValueDataAsEnum<DroneState>() == READY || droneState->getValueDataAsEnum<DroneState>() == STABILIZING)
			&& isUpsideDown
			&& realPosition->y < .3f;//if detected flying high, do not do anything !

		if (upsideDownAndShouldDoSomething)
		{
			if (autoKillUpsideDown->boolValue())
			{
				NLOGWARNING(niceName, "Drone Upside down and appears close to ground, kill");
				droneState->setValueWithData(ERROR);
				lightMode->setValueWithKey("Off");
				headlight->setValue(false);
			} else if (freezeOnUpsideDown->boolValue())
			{
				upsideDownFrozen = true;
			}
		} else
		{
			upsideDownFrozen = false;
		}
	}
}

// PARAMETERS AND TRIGGERS
void Drone::onContainerParameterChangedInternal(Parameter * p)
{
	//before enable to be able to change light on disable
	if (p == headlight) setParam("ring", "headlightEnable", headlight->boolValue());
	else if (p == lightMode) setParam("ring", "effect", (int)lightMode->getValueData());
	else if (p == color)
	{
		setParam("ring", "solidRed", color->getColor().getRed());
		setParam("ring", "solidGreen", color->getColor().getGreen());
		setParam("ring", "solidBlue", color->getColor().getBlue());
	}

	if (p == enabled)
	{
		if (!enabled->boolValue())
		{
			droneState->setValueWithData(DISCONNECTED);
			headlight->setValue(false);
			lightMode->setValueWithKey("Off");
			targetPosition->setValue(realPosition->x, 0, realPosition->z);
		} else
		{
			isFlying->setValue(false);
			lowBattery->setValue(false);
			charging->setValue(false);
		}
	}

	if (!enabled->boolValue()) return;


	if (cf == nullptr) return;
	
	if (p == launchingMode)
	{
		if (launchingMode->boolValue())
		{
			absoluteMode->setValue(false);
			NLOG(niceName, "Launching drone for " << DroneManager::getInstance()->launchTime->floatValue() << " s");
		} else
		{
			NLOG(niceName, "Launching Complete, sync full position");
			absoluteMode->setValue(true);
		}

		timeAtLaunch = Time::getMillisecondCounter() / 1000.0f;
		
	}

	if (p == realPosition)
	{
		if (launchingMode->boolValue()) targetPosition->setVector(realPosition->x, realPosition->y, realPosition->z);
		isFlying->setValue(realPosition->y > .1f);
	}
	

	if (p == absoluteMode)
	{
		NLOG(niceName, "Set absolute position mode");
		setParam("flightmode", "posSet", absoluteMode->intValue());
	}

	if (p == droneState)
	{
		switch (droneState->getValueDataAsEnum<DroneState>())
		{
		case DISCONNECTED:
			lowBattery->setValue(false);
			yaw->setValue(0);
			break;

		case ERROR: lightMode->setValueWithKey("Alert"); break;
		case STABILIZING: 
			color->setColor(Colours::white); //to force ready state to send value
			lightMode->setValueWithKey("Double spinner");
			yaw->setValue(0);
			break;
		case READY: 
			targetPosition->setVector(realPosition->x, 0, realPosition->z);
			color->setColor(Colours::black);
			yaw->setValue(0);
			lightMode->setValueWithKey("Solid color");
			break;
            default:
                break;
		}
	}

	if(droneState->getValueDataAsEnum<DroneState>() != READY) return;
	
	if (p == targetPosition) setTargetPosition(targetPosition->x, targetPosition->y, targetPosition->z, yaw->floatValue());

	if (p == voltage)
	{
		if (voltage->floatValue() > 0 && !charging->boolValue()) //ensure voltage has been set and drone is not charging
		{
			if (!lowBattery->boolValue())
			{
				bool isNowLowBattery = false;
				if (targetPosition->y == 0) isNowLowBattery =  voltage->floatValue() <= DroneManager::getInstance()->onGroundLowBatteryThreshold->floatValue(); //drone is not flying, min battery is 3.1V
				else  isNowLowBattery = voltage->floatValue() <= DroneManager::getInstance()->flyingLowBatteryThreshold->floatValue(); //drone is flying, min battery is 2.7V

				if (isNowLowBattery)
				{
					float currentTime = Time::getMillisecondCounter() / 1000.0f;
					if (timeAtBelowLowBattery == 0) timeAtBelowLowBattery = currentTime;
					else if (currentTime > timeAtBelowLowBattery + DroneManager::getInstance()->lowBatteryTimeCheck->floatValue())
					{
						lowBattery->setValue(true);
					}
				} else
				{
					timeAtBelowLowBattery = 0;
				}
			}
		}
		else
		{
			lowBattery->setValue(false);
		}
	} else if (p == charging)
	{
		lowBattery->setValue(false);
	} 
	/*else if (p == kd)
	{
		NLOG(niceName, "Update Kd PID Settings");
		setParam("posCtlPid", "xKd", kd->x);
		setParam("posCtlPid", "yKd", kd->y);
		setParam("posCtlPid", "zKd", kd->z);
	} else if (p == ki)
	{
		NLOG(niceName, "Update Ki Settings");
		setParam("posCtlPid", "xKi", ki->x);
		setParam("posCtlPid", "yKi", ki->y);
		setParam("posCtlPid", "zKi", ki->z);
	} else if (p == kp)
	{
		NLOG(niceName, "Update Kp Settings");
		setParam("posCtlPid", "xKp", kp->x);
		setParam("posCtlPid", "yKp", kp->y);
		setParam("posCtlPid", "zKp", kp->z);
	} else if (p == xyVelMax)
	{
		setParam("posCtlPid", "xyVelMax", xyVelMax->floatValue());
	} else if (p == zVelMax)
	{
		
		setParam("posCtlPid", "zVelMax", zVelMax->floatValue());
	} else if (p == rpLimit)
	{
		setParam("posCtlPid", "rpLimit", rpLimit->floatValue());		
	}
	*/

}

void Drone::onContainerTriggerTriggered(Trigger * t)
{
	if (!enabled->boolValue()) return;

	if (t == connectTrigger) launchCFThread();
	if (cf == nullptr) return;
	
	if (t == resetKalmanTrigger)
	{
		//NLOG(address->stringValue(), "Reset Kalman Estimation");

		timeAtResetKalman = Time::getApproximateMillisecondCounter() / 1000.0f;
		droneState->setValueWithData(STABILIZING);

		setParam("kalman", "resetEstimation", 1);
		sleep(2);
		setParam("kalman", "resetEstimation", 0);
	} 


	if(droneState->getValueDataAsEnum<DroneState>() != READY && droneState->getValueDataAsEnum<DroneState>() != STABILIZING) return;
	
	if (t == taskDump) setParam("system", "taskDump", 1);
	else if (t == logParamsTOC)  logAllParams();
	else if (t == logLogsTOC)  logAllLogs();
	else if (t == launchTrigger) launchingMode->setValue(true);// targetPosition->setVector(targetPosition->x, .4f, targetPosition->z);
	else if (t == stopTrigger) targetPosition->setVector(targetPosition->x, 0, targetPosition->z);
	else if (t == syncTrigger) targetPosition->setVector(realPosition->x, 0, realPosition->z);
	else if (t == writeToAnchors) setAnchors(NodeManager::getInstance()->getAllPositions());
}


//CONNECTION
bool Drone::setupCF()
{
	NLOG(address->stringValue(), "Connect " << getRadioString() << "...");
	droneState->setValueWithData(CONNECTING);

	
	try
	{
		dataLogBlock = nullptr; 
		feedbackBlock = nullptr;

		cf = nullptr; //delete previous
		cf = new Crazyflie(targetRadio->intValue(), channel->intValue(), speed->getValueDataAsEnum<Crazyradio::Datarate>(), address->stringValue());

		droneHasStarted = false;
		droneHasFinishedInit = false;
		cf->reboot();

		std::function<void(const char *)> consoleF = std::bind(&Drone::consoleCallback, this, std::placeholders::_1);
		cf->setConsoleCallback(consoleF);

		std::function<void(const crtpPlatformRSSIAck *)> ackF = std::bind(&Drone::emptyAckCallback, this, std::placeholders::_1);
		cf->setEmptyAckCallback(ackF);

		std::function<void(float)> linkF = std::bind(&Drone::linkQualityCallback, this, std::placeholders::_1);
		cf->setLinkQualityCallback(linkF);

		
		float initStartTime = Time::getApproximateMillisecondCounter() / 1000.0f;
		while (!droneHasFinishedInit && !threadShouldExit() && droneState->getValueDataAsEnum<DroneState>() == CONNECTING)
		{
			cf->sendPing();
			sleep(20);
			if (Time::getApproximateMillisecondCounter() / 1000.0f > initStartTime + 5) break; //max 5 seconds to init the drone, otherwise try to stabilize
		}
		

		NLOG(address->stringValue(), "Init finished, setup params and logs");
		cf->requestParamToc();

		uint8 selfTest = cf->getParam<uint8>(cf->getParamTocEntry("system", "selftestPassed")->id);

		if (!selfTest)
		{
			NLOG(address->stringValue(), "SelfTest failed.");
			droneState->setValueWithData(ERROR);
			return false;
		}

		NLOG(address->stringValue(), "SelfTest check passed");

		cf->logReset();
		cf->requestLogToc();

		std::function<void(uint32_t, dataLog *)> cb = std::bind(&Drone::dataLogCallback, this, std::placeholders::_1, std::placeholders::_2);
		dataLogBlock = new LogBlock<dataLog>(cf, { { "pm","vbat" },{"pm","state"},{ "kalman","stateX" },{ "kalman","stateY" },{ "kalman","stateZ" } }, cb);
		dataLogBlock->start(10); // 50ms - 20fps

		std::function<void(uint32_t, feedbackLog *)> fcb = std::bind(&Drone::feedbackLogCallback, this, std::placeholders::_1, std::placeholders::_2);
		feedbackBlock = new LogBlock<feedbackLog>(cf, { { "stabilizer","pitch" },{ "stabilizer","yaw" },{ "stabilizer","roll" }}, fcb);
		feedbackBlock->start(10); // 50ms - 20fps

		realPosition->setVector(0, 0, 0);
		targetPosition->setVector(0, 0, 0);
		
		lightMode->setValueWithKey("Solid color");
		
		//NLOG(address->stringValue(), "Disable thrust lock.");
		for (int i = 0; i < 10; i++) cf->sendSetpoint(0, 0, 0, 0); // disable thrust lock, put in autoArm param ?
		
		
		/*
		if (initPIDSettings->boolValue())
		{
			//TODO : ask why ????
			setParam("posCtlPid", "xKp", kp->x);
			setParam("posCtlPid", "yKp", kp->y);
			setParam("posCtlPid", "zKp", kp->z);
			sleep(20);
			setParam("posCtlPid", "xKi",  ki->x);
			setParam("posCtlPid", "yKi",  ki->y);
			setParam("posCtlPid", "zKi", ki->z);
			sleep(20);
			setParam("posCtlPid", "xKd", kd->x);
			setParam("posCtlPid", "yKd", kd->y);
			setParam("posCtlPid", "zKd", kd->z);
			sleep(20);
			setParam("posCtlPid", "rpLimit", rpLimit->floatValue());
			setParam("posCtlPid", "xyVelMax", xyVelMax->floatValue());
			setParam("posCtlPid", "zVelMax", zVelMax->floatValue());
			//
		}
		*/

		if (initAnchorPos->boolValue())
		{
			//NLOG(address->stringValue(), "Set anchor positions");
			setAnchors(NodeManager::getInstance()->getAllPositions());
		}else
		{
			sleep(500);
			resetKalmanTrigger->trigger();
		}

		
		absoluteMode->setValue(true);

		lowBattery->setValue(false); //reset low battery on connect

		NLOG(address->stringValue(), "Connected.");
		//droneState->setValueWithData(READY);

	}
	catch (std::runtime_error &e)
	{
		NLOG(address->stringValue(), "Could not connect : " << e.what());
		droneState->setValueWithData(DISCONNECTED);
		return false;
	}

	return true;
}

void Drone::selfTestCheck()
{
	uint8 selfTest = true;
	
	{
		const SpinLock::ScopedLockType lock(cfLock);
		selfTest = cf->requestParamValue<uint8>(cf->getParamTocEntry("system", "selftestPassed")->id);
	}

	outTrigger->trigger();

	if (!selfTest)
	{
		NLOG(address->stringValue(), "SelfTest failed.");
		droneState->setValueWithData(ERROR);
	}
}


template <class T>
bool Drone::setParam(String group, String paramID, T value)
{
	if (cf == nullptr) return false;

	SpinLock::ScopedLockType lock(cfLock);
	try
	{
		const Crazyflie::ParamTocEntry * pte = cf->getParamTocEntry(group.toStdString(), paramID.toStdString());
		
		if (pte == nullptr)
		{
			if(enableLogParams->boolValue()) NLOG(address->stringValue(), "### Could not find param Entry for " + group + "." + paramID);
			return false;
		}

		cf->setParam(pte->id, value);
		outTrigger->trigger();

		if (enableLogParams->boolValue()) NLOG(address->stringValue(), "Set Param : " << group << "." << paramID << " : " << (float)value);
	}
	catch (std::runtime_error &e)
	{
		if (enableLogParams->boolValue()) NLOG(address->stringValue(), "Error setting param " << group << "." << paramID <<" : " << e.what());
		return false;
	}

	return true;
}

bool Drone::setTargetPosition(float x, float y, float z, float _yaw, bool showTrigger)
{
	if (cf == nullptr || droneState->getValueDataAsEnum<DroneState>() != READY) return false;
	if (upsideDownFrozen) return false;
	if (!absoluteMode->boolValue()) return false;

	//setParam("flightmode", "posSet", 1);
	SpinLock::ScopedLockType lock(cfLock);
	try
	{
#if DEBUG
		if (enableLogParams->boolValue()) DBG("Send set point " << x<< " / " << y << " / " << z);
#endif
		cf->sendSetpoint(z,-x, _yaw, (uint16_t)(y * 1000)); //z is height for the drones, height is y for me.
		outTrigger->trigger();
	}
	catch (std::runtime_error &e)
	{
		if (enableLogParams->boolValue()) NLOG(address->stringValue(),"Error setting target Position : " << e.what());
		return false;
	}
	
	return true;
}

// COMMANDS
bool Drone::setAnchors(Array<Vector3D<float>> positions)
{

	if (cf == nullptr) return false;
	
	{

		//NLOG(address->stringValue(), "Set anchors : " << positions.size() << " values");
		try
		{
			for (int i = 0; i < positions.size() && i < 8; i++)
			{
				//NLOG(address->stringValue(), "Set anchor (drone order)" + String(i) + " : " + String(positions[i].x) + " , " + String(positions[i].z) + ", " + String(positions[i].y));
				//setParam("anchorpos", (String("anchor") + String(i) + String("x")).toStdString(), positions[i].x);
				//setParam("anchorpos", (String("anchor") + String(i) + String("y")).toStdString(), positions[i].z); //z is height for the drones, height is y for me.
				//setParam("anchorpos", (String("anchor") + String(i) + String("z")).toStdString(), positions[i].y);

				cf->sendSetNodePos(i, positions[i].x, positions[i].z, positions[i].y); //z is up for the drone, y is up for me
			}

			//setParam("anchorpos", "enable", 1);
		}

		catch (std::runtime_error &e)
		{
			if (enableLogParams->boolValue()) NLOG(address->stringValue(), "Error setting anchors : " << e.what());
			return false;
		}
	}

	resetKalmanTrigger->trigger();

	return true; 
	
}

void Drone::logAllParams()
{
	
	try
	{
		{ SpinLock::ScopedLockType lock(cfLock);  cf->requestParamToc(); }
		const String types[]{ "?","uint8","uint16","uint32","int8","int16","int32","float" };

		LOG("Param Entries :");
		std::for_each(cf->paramsBegin(), cf->paramsEnd(),
			[&](const Crazyflie::ParamTocEntry& entry) { 
				LOG(String(entry.group) << "." << String(entry.name)); // << " (" << types[(int)entry.type] << ")" << (entry.readonly ? " readyonly" : ""));
			});
	}
#if DEBUG
	catch (std::exception& e)
#else
	catch(std::exception& )
#endif
	{
		DBG("Error getting params  : " << e.what() << "");
	}
}

void Drone::logAllLogs()
{
	try
	{
		//scope for releasing the lock after the request
		{ SpinLock::ScopedLockType lock(cfLock); cf->requestLogToc(); }

		LOG("Log Variable Entries :");
		const String types[]{ "?","uint8","uint16","uint32","int8","int16","int32","float" };

		std::for_each(cf->logVariablesBegin(), cf->logVariablesEnd(),
			[&](const Crazyflie::LogTocEntry& entry) { LOG(String(entry.group) + "." + String(entry.name) + " (" + types[(int)entry.type] + ")"); });
	}
#if DEBUG
	catch (std::exception& e)
#else
	catch(std::exception &)
#endif
	{
		DBG("Error getting logs  : " << e.what() << "");
	}

}



//CALLBACKS
void Drone::consoleCallback(const char * c)
{
	lastAckTime = Time::getApproximateMillisecondCounter();

	consoleBuffer += String(c);

	if (consoleBuffer.endsWith("\n"))
	{
		if (enableLogConsole->boolValue())
		{
			if (consoleBuffer.toLowerCase().contains("error") || consoleBuffer.toLowerCase().contains("fail")) NLOGERROR(address->stringValue(), consoleBuffer);
			else NLOG(address->stringValue(), consoleBuffer);
		}

		if (consoleBuffer.contains("----------------------------"))
		{
			//DBG("Drone has started");
			if (droneHasStarted)
			{
				//drone has already started, it means that drone has started again by itself. what to do here ?
				
			}
			else droneHasStarted = true;
		}
		else if (consoleBuffer.contains("Free heap") || (consoleBuffer.contains("DWM") && consoleBuffer.contains("TDoA")) || consoleBuffer.contains("Deck 1 Test [OK]"))
		{
			//DBG("Drone init finish");
			droneHasFinishedInit = true;
		}
		else if (consoleBuffer.contains("Assert failed") || consoleBuffer.contains("[FAIL]"))
		{
			droneState->setValueWithData(ERROR);
		}

		consoleBuffer.clear();
	}
}

void Drone::emptyAckCallback(const crtpPlatformRSSIAck * a)
{
	lastAckTime = Time::getApproximateMillisecondCounter();
	inTrigger->trigger();
}

void Drone::linkQualityCallback(float val)
{
	linkQuality->setValue(val);
}

void Drone::dataLogCallback(uint32_t, dataLog * data)
{
	voltage->setValue(data->battery);
	realPosition->setVector(data->x, data->z, data->y); //z is height for the drones, height is y for me.
	charging->setValue(data->charging == 1);
}

void Drone::feedbackLogCallback(uint32_t, feedbackLog * data)
{
	orientation->setVector(data->pitch, data->yaw, data->roll);
}


//THREAD
void Drone::run()
{
	//DBG("Drone thread start")

	while (enabled->boolValue() && !threadShouldExit())
	{
		//Connect
		bool result = setupCF();

		//If connected, loop while drone is OK
		if (result)
		{
			uint32 lastSelfTestCheck = Time::getApproximateMillisecondCounter();
			uint32 lastPosSend = Time::getApproximateMillisecondCounter();

			//Stabilization check variables
			const float stabilizedDistanceThreshold = .1f;
			const int numDistChecked = 10;
			float distances[numDistChecked];
			float averageDistance = 1000;
			for (int i = 0; i < numDistChecked; i++) distances[i] = 1000;
			int distIndex = 0;
			Vector3D<float> lastRealPos;
			uint32 firstStabilizedTime = Time::getApproximateMillisecondCounter();
			timeAtResetKalman = Time::getApproximateMillisecondCounter() / 1000.0f;

			while (enabled->boolValue() && !threadShouldExit() && droneState->getValueDataAsEnum<DroneState>() != ERROR)
			{
				sleep(100);

				//ping, and stuff here
				try
				{
					{ 
						const SpinLock::ScopedLockType lock(cfLock);
						for(int i=0;i<10;i++) cf->sendPing();
					}
				
					uint32 time = Time::getApproximateMillisecondCounter();
					if (time > lastSelfTestCheck + 3000)
					{
						selfTestCheck();
						lastSelfTestCheck = time;
					}

					if (time > lastPosSend + 200) // send pos every 200ms and check stab
					{
						if (absoluteMode->boolValue())
						{
							setTargetPosition(targetPosition->x, targetPosition->y, targetPosition->z, yaw->floatValue());
						} else if(launchingMode->boolValue())
						{
							cf->sendSetpoint(0, 0, 0, DroneManager::getInstance()->launchForce->floatValue()*10000);
							if (time/1000.0f > timeAtLaunch + DroneManager::getInstance()->launchTime->floatValue()) launchingMode->setValue(false);
							
						}

						lastPosSend = time;

						if (droneState->getValueDataAsEnum<DroneState>() == STABILIZING)
						{
							

							/*
							//If more than 5 seconds to stabilize, try to restabilize
							if (Time::getApproximateMillisecondCounter() / 1000.0f > timeAtResetKalman + 5)
							{
								NLOGWARNING(niceName, "Stabilization takes too much time, reset kalman again");
								resetKalmanTrigger->trigger();
								continue;
							}
							*/

							//STABILIZATION

							Vector3D<float> curRealPos = realPosition->getVector();
							if (curRealPos.x != .5 && curRealPos.y != .5 && fabsf(curRealPos.x) != 10 && fabs(curRealPos.z) != 10)
							{
								//targetPosition->setVector(curRealPos.x, 0, curRealPos.z);

								float dist = sqrtf((curRealPos.x - lastRealPos.x)*(curRealPos.x - lastRealPos.x) + (curRealPos.y - lastRealPos.y)*(curRealPos.y - lastRealPos.y) + (curRealPos.z - lastRealPos.z)*(curRealPos.z - lastRealPos.z));
								distances[distIndex] = dist;
								distIndex = (distIndex + 1) % numDistChecked;
								float avDist = 0;
								for (int i = 0; i < numDistChecked; i++) avDist += distances[i] / numDistChecked;

								lastRealPos = curRealPos;

								//DBG("Stabilizing, dist = " << avDist);
								if (avDist < stabilizedDistanceThreshold)
								{
									if (averageDistance > stabilizedDistanceThreshold) //if just passed below the threshold
									{
										firstStabilizedTime = Time::getApproximateMillisecondCounter();
									}

									if (time > firstStabilizedTime + 200) //got full stab for 1 second
									{
										droneState->setValueWithData(READY);
									}

									averageDistance = avDist;
								}
							}
						}
					}

				}
				catch (std::runtime_error &)
				{
					//NLOG(address->stringValue(), "Routine failed : " << e.what());
					droneState->setValueWithData(ERROR);
				}

			}
		}

		//If drone is in error mode or could not connect, if autoReconnect, will wait 1 second and try reconnecting, else it will end the thread
		if (!autoReconnect->boolValue() || threadShouldExit() || !enabled->boolValue()) return;
		sleep(800);
	}

	//DBG("Drone Thread finish");
}
