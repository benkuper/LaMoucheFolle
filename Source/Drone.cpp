/*
  ==============================================================================

	Drone.cpp
	Created: 19 Oct 2017 7:33:14pm
	Author:  Ben

  ==============================================================================
*/

#include "Drone.h"
#include "NodeManager.h"

Drone::Drone() :
	BaseItem("Drone"),
	cf(nullptr),
	ackTimeout(2000), //2s no packet = drone disconnected
	Thread("DroneInitThread")
{
	targetRadio = addIntParameter("Radio", "Target Radio to connect", 0, 0, 16);
	channel = addIntParameter("Channel", "Target channel of the drone", 40, 0, 200);
	speed = addEnumParameter("Baudrate", "Speed of the connection");
	speed->addOption("250K", Crazyradio::Datarate_250KPS)->addOption("1M", Crazyradio::Datarate_1MPS)->addOption("2M", Crazyradio::Datarate_2MPS);

	address = addStringParameter("Address", "Address of the drone, without the 0x", "E7E7E7E7E7");

	targetPosition = addPoint3DParameter("Target", "Target Position for the drone");
	targetPosition->setBounds(0,0,0, 10, 10, 10);

	realPosition = addPoint3DParameter("Real Position", "Position feedback from the drone");
	realPosition->setBounds(0,0,0, 10, 10, 10);

	goalFeedback = addPoint3DParameter("Target", "Target Position for the drone");
	goalFeedback->setBounds(0, 0, 0, 10, 10, 10);

	absoluteMode = addBoolParameter("Absolute Mode", "Absolute positionning", false);

	yaw = addFloatParameter("Yaw", "Horizontal rotation of the drone", 0, 0, 360);

	droneState = addEnumParameter("State", "Drone State");
	droneState->addOption("Disconnected", DISCONNECTED)->addOption("Connecting", CONNECTING)->addOption("Stabilizing",STABILIZING)->addOption("Ready", READY)->addOption("Error", ERROR);
	droneState->isEditable = false;
	droneState->isSavable = false;

	connectTrigger = addTrigger("Connect", "Init the connection to the radio");

	logParams = addTrigger("Log Params", "Log all params from the drone");
	logLogs = addTrigger("Log Logs", "Log logs entries from the drone");

	taskDump = addTrigger("TaskDump", "TD");

	launchTrigger = addTrigger("Launch", "Launch");
	stopTrigger = addTrigger("Stop", "Stop");
	syncTrigger = addTrigger("Sync Position", "Sync position");

	resetKalmanTrigger = addTrigger("Reset Kalman Estimation", "Reset Kalman Filter Estimation");

	lightMode = addEnumParameter("LightMode", "Led Preset");
	lightMode->addOption("Off", 0)->addOption("White spinner", 1)->addOption("Color spinner",2)->addOption("Tilt effect", 3) \
		->addOption("Brightness", 4)->addOption("Color spinner2", 5)->addOption("Double spinner", 6)->addOption("Solid color", 7) \
		->addOption("Factory test", 8)->addOption("Battery status", 9)->addOption("Boat lights", 10)->addOption("Alert", 11)->addOption("Gravity", 12);


	color = new ColorParameter("Light Color", "LightColor", Colours::red);
	addParameter(color);

	headlight = addBoolParameter("Headlight", "Headlight", false);

	autoReconnect = addBoolParameter("Auto Reconnect", "Auto Reboot drone if connected but in bad state", false);

	linkQuality = addFloatParameter("Link Quality", "Quality of radio communication with the drone", 0, 0, 1);
	linkQuality->isEditable = false;
	voltage = addFloatParameter("Voltage", "Voltage of the drone. /!\\ When flying, there is a massive voltage drop !", 0, 0, 4.2f);
	voltage->isEditable = false;
	
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
	waitForThreadToExit(3000);
}



// PARAMETERS AND TRIGGERS
void Drone::onContainerParameterChangedInternal(Parameter * p)
{
	if (p == enabled)
	{
		if (!enabled->boolValue()) droneState->setValueWithData(DISCONNECTED);
	}

	if (!enabled->boolValue()) return;
	if (cf == nullptr) return;
	

	if (p == realPosition)
	{

	}

	if (p == absoluteMode)
	{
		NLOG(address->stringValue(), "Set absolute position mode");
		setParam("flightmode", "posSet", absoluteMode->intValue());
	}

	if (p == droneState)
	{
		switch (droneState->getValueDataAsEnum<DroneState>())
		{
		case ERROR: lightMode->setValueWithKey("Alert"); break;
		case STABILIZING: lightMode->setValueWithKey("Double spinner"); break;
		case READY: 
			targetPosition->setVector(realPosition->x, 0, realPosition->z);
			lightMode->setValueWithKey("Off"); 
			break;
		}
	}

	if (p == headlight) setParam("ring", "headlightEnable", headlight->boolValue());
	if (p == lightMode) setParam("ring", "effect", (int)lightMode->getValueData());


	if(droneState->getValueDataAsEnum<DroneState>() != READY) return;
	
	else if (p == color)
	{
		setParam("ring", "solidRed", color->getColor().getRed());
		setParam("ring", "solidGreen", color->getColor().getGreen());
		setParam("ring", "solidBlue", color->getColor().getBlue());
	}else if (p == targetPosition) setTargetPosition(targetPosition->x, targetPosition->y, targetPosition->z, yaw->floatValue());

	if (p == voltage)
	{
		if (voltage->floatValue() > 0 && !charging->boolValue()) //ensure voltage has been set and drone is not charging
		{
			if(targetPosition->y == 0) lowBattery->setValue(voltage->floatValue() < 3.1f); //drone is not flying, min battery is 3.1V
			else lowBattery->setValue(voltage->floatValue() < 2.7f); //drone is flying, min battery is 2.7V
		}
		else
		{
			lowBattery->setValue(false);
		}
	}
}

void Drone::onContainerTriggerTriggered(Trigger * t)
{
	if (!enabled->boolValue()) return;

	if (t == connectTrigger) launchCFThread();
	if (cf == nullptr) return;
	
	if (t == resetKalmanTrigger)
	{
		NLOG(address->stringValue(), "Reset Kalman Estimation");
		droneState->setValueWithData(STABILIZING);
		setParam("kalman", "resetEstimation", 1);
		sleep(2);
		setParam("kalman", "resetEstimation", 0);
	} 
	


	if(droneState->getValueDataAsEnum<DroneState>() != READY && droneState->getValueDataAsEnum<DroneState>() != STABILIZING) return;
	
	if (t == taskDump) setParam("system", "taskDump", 1);
	else if (t == logParams)  logAllParams();
	else if (t == logLogs)  logAllLogs();
	else if (t == launchTrigger) targetPosition->setVector(targetPosition->x, .4f, targetPosition->z);
	else if (t == stopTrigger) targetPosition->setVector(targetPosition->x, 0, targetPosition->z);
	else if (t == syncTrigger) targetPosition->setVector(realPosition->x, 0, realPosition->z);
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

		while (!droneHasFinishedInit && droneState->getValueDataAsEnum<DroneState>() == CONNECTING)
		{
			cf->sendPing();
			sleep(10);
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
		feedbackBlock = new LogBlock<feedbackLog>(cf, { { "posCtl","targetX" },{ "posCtl","targetY" },{ "posCtl","targetZ" }}, fcb);
		feedbackBlock->start(10); // 50ms - 20fps


		
		realPosition->setVector(0, 0, 0);
		targetPosition->setVector(0, 0, 0);
		
		lightMode->setValueWithKey("Off");
		
		NLOG(address->stringValue(), "Disable thrust lock.");
		for (int i = 0; i < 10; i++) cf->sendSetpoint(0, 0, 0, 0); // disable thrust lock, put in autoArm param ?

		setParam("flightmode", "posSet", 1);
		

		//TODO : ask why ????
		setParam("posCtlPid", "xKp", 1.5f);
		setParam("posCtlPid", "yKp", 1.5f);
		setParam("posCtlPid", "zKp", 1.5f);

		setParam("posCtlPid", "xKi", 0.25f);
		setParam("posCtlPid", "yKi", 0.25f);
		setParam("posCtlPid", "zKi", 0.5f);
		//



		NLOG(address->stringValue(), "Set anchor positions");
		setAnchors(NodeManager::getInstance()->getAllPositions());

		absoluteMode->setValue(true);


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
			NLOG(address->stringValue(), "### Could not find param Entry for " + group + "." + paramID);
			return false;
		}

		cf->setParam(pte->id, value);
		outTrigger->trigger();

		NLOG(address->stringValue(), "** SET PARAM " << group << "." << paramID << " : " << (float)value);
	}
	catch (std::runtime_error &e)
	{
		NLOG(address->stringValue(), "Error setting param " << group << "." << paramID <<" : " << e.what());
		return false;
	}

	return true;
}

bool Drone::setTargetPosition(float x, float y, float z, float _yaw, bool showTrigger)
{
	if (cf == nullptr || droneState->getValueDataAsEnum<DroneState>() != READY) return false;

	//setParam("flightmode", "posSet", 1);

	SpinLock::ScopedLockType lock(cfLock);
	try
	{
		DBG("Send set point " << x<< " / " << y << " / " << z);
		cf->sendSetpoint(z,-x, _yaw, (uint16_t)(y * 1000)); //z is height for the drones, height is y for me.
		outTrigger->trigger();
	}
	catch (std::runtime_error &e)
	{
		NLOG(address->stringValue(),"Error setting target Position : " << e.what());
		return false;
	}
	
	return true;
}

// COMMANDS
bool Drone::setAnchors(Array<Vector3D<float>> positions)
{

	if (cf == nullptr) return false;
	
	{

		NLOG(address->stringValue(), "Set anchors : " << positions.size() << " values");
		try
		{
			for (int i = 0; i < positions.size() && i < 8; i++)
			{
				NLOG(address->stringValue(), "Set anchor (drone order)" + String(i) + " : " + String(positions[i].x) + " , " + String(positions[i].z) + ", " + String(positions[i].y));
				setParam("anchorpos", (String("anchor") + String(i) + String("x")).toStdString(), positions[i].x);
				setParam("anchorpos", (String("anchor") + String(i) + String("y")).toStdString(), positions[i].z); //z is height for the drones, height is y for me.
				setParam("anchorpos", (String("anchor") + String(i) + String("z")).toStdString(), positions[i].y);
			}


			//setParam("anchorpos", "enable", 1);
		}
		catch (std::runtime_error &e)
		{
			NLOG(address->stringValue(), "Error setting anchors : " << e.what());
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
			[&](const Crazyflie::ParamTocEntry& entry) { LOG(String(entry.group) << "." << String(entry.name));// << " (" << types[(int)entry.type] << ")" << (entry.readonly ? " readyonly" : ""));
	});
	}
	catch (std::exception& e)
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
	catch (std::exception& e)
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
		NLOG(address->stringValue(), consoleBuffer);

		if (consoleBuffer.contains("----------------------------"))
		{
			DBG("Drone has started");
			if (droneHasStarted)
			{
				//drone has already started, it means that drone has started again by itself. what to do here ?
				
			}
			else droneHasStarted = true;
		}
		else if (consoleBuffer.contains("Free heap"))
		{
			DBG("Drone init finish");
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
	goalFeedback->setVector(data->goalX, data->goalZ, data->goalY);
}


//THREAD
void Drone::run()
{
	DBG("Drone thread start");

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
						setTargetPosition(targetPosition->x, targetPosition->y, targetPosition->z,yaw->floatValue());
						lastPosSend = time;

						if (droneState->getValueDataAsEnum<DroneState>() == STABILIZING)
						{
							Vector3D<float> curRealPos = realPosition->getVector();
							if (curRealPos.x != lastRealPos.x || curRealPos.y != lastRealPos.y || curRealPos.z != lastRealPos.z)
							{
								//targetPosition->setVector(curRealPos.x, 0, curRealPos.z);

								float dist = sqrtf((curRealPos.x - lastRealPos.x)*(curRealPos.x - lastRealPos.x) + (curRealPos.y - lastRealPos.y)*(curRealPos.y - lastRealPos.y) + (curRealPos.z - lastRealPos.z)*(curRealPos.z - lastRealPos.z));
								distances[distIndex] = dist;
								distIndex = (distIndex + 1) % numDistChecked;
								float avDist = 0;
								for (int i = 0; i < numDistChecked; i++) avDist += distances[i] / numDistChecked;

								lastRealPos = curRealPos;

								DBG("Stabilizing, dist = " << avDist);
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
				catch (std::runtime_error &e)
				{
					NLOG(address->stringValue(), "Routine failed : " << e.what());
					droneState->setValueWithData(ERROR);
				}

			}
		}

		//If drone is in error mode or could not connect, if autoReconnect, will wait 1 second and try reconnecting, else it will end the thread
		if (!autoReconnect->boolValue() || threadShouldExit() || !enabled->boolValue()) return;
		sleep(3000);
	}

	DBG("Drone Thread finish");
}
