/*
  ==============================================================================

    Drone2.cpp
    Created: 12 May 2018 5:18:44pm
    Author:  Ben

  ==============================================================================
*/

#include "Drone2.h"
#include "DroneManager.h"

Drone2::Drone2() :
	BaseItem("Drone"),
	Thread("DroneThread"),
	timeAtStartTakeOff(0),
	timeAtStartLanding(0),
	timeAtStartConverge(0),
	timeAtBelowLowBattery(0)
{
	targetRadio = addIntParameter("Radio", "Target Radio to connect", 0, 0, 16);
	channel = addIntParameter("Channel", "Target channel of the drone", 40, 0, 200);
	speed = addEnumParameter("Baudrate", "Speed of the connection");
	speed->addOption("2M", Crazyradio::Datarate_2MPS)->addOption("250K", Crazyradio::Datarate_250KPS)->addOption("1M", Crazyradio::Datarate_1MPS);

	address = addStringParameter("Address", "Address of the drone, without the 0x", "E7E7E7E7E7");

	state = addEnumParameter("State", "State of the drone");
	state->addOption("Disconnected", DISCONNECTED)->addOption("Connecting", CONNECTING)->addOption("Calibrating", CALIBRATING)
		 ->addOption("Ready",READY)->addOption("Taking off", TAKEOFF)->addOption("Flying", FLYING)->addOption("Landing", LANDING)
		 ->addOption("Error", ERROR)->addOption("Booting", BOOTING);

	state->isEditable = false;
	state->isSavable = false;

	connectTrigger = addTrigger("Connect", "Init the connection to the radio");
	calibrateTrigger = addTrigger("Calibrate", "Reset Kalman Estimation");
	takeOffTrigger = addTrigger("Take Off", "Take off");
	unlockTrigger = addTrigger("Unlock", "Put them in flying mode without takeoff");

	landTrigger = addTrigger("Land", "Land");
	rebootTrigger = addTrigger("Reboot", "Reboot the drone");


	realPosition = addPoint3DParameter("Real Position", "Real Position feedback from the drone");
	realPosition->setBounds(-10, -1, -10, 10, 10, 10);
	realPosition->isEditable = false;
	realPosition->isSavable = false;

	targetPosition = addPoint3DParameter("Target", "Target Position for the drone");
	targetPosition->setBounds(-10,0,-10,10,10,10);
	targetPosition->isSavable = false;

	lightMode = addEnumParameter("LightMode", "Led Preset");
	lightMode->addOption("Off", 0)->addOption("White spinner", 1)->addOption("Color spinner", 2)->addOption("Tilt effect", 3) \
		->addOption("Brightness", 4)->addOption("Color spinner2", 5)->addOption("Double spinner", 6)->addOption("Solid color", 7) \
		->addOption("Factory test", 8)->addOption("Battery status", 9)->addOption("Boat lights", 10)->addOption("Alert", 11)->addOption("Gravity", 12);
	lightMode->isSavable = false;

	color = new ColorParameter("Light Color", "LightColor", Colours::black);
	addParameter(color);
	color->isSavable = false;

	headlight = addBoolParameter("Headlight", "Headlight", false);
	headlight->isSavable = false;

	linkQuality = addFloatParameter("Link Quality", "Quality of radio communication with the drone", 0, 0, 1);
	linkQuality->isEditable = false;
	linkQuality->isSavable = false;

	voltage = addFloatParameter("Voltage", "Voltage of the drone. /!\\ When flying, there is a massive voltage drop !", 0, 0, 4.4f);
	voltage->setRange(DroneManager::getInstance()->flyingLowBatteryThreshold->floatValue(), voltage->maximumValue);
	voltage->isEditable = false;
	voltage->isSavable = false;

	lowBattery = addBoolParameter("Low Battery", "Low battery (measured when not flying)", false);
	lowBattery->isEditable = false;
	lowBattery->isSavable = false;


	startThread();
}

Drone2::~Drone2()
{
	signalThreadShouldExit();
	waitForThreadToExit(1000);
}


void Drone2::onContainerParameterChangedInternal(Parameter * p)
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
		}
	}
}

void Drone2::onContainerTriggerTriggered(Trigger * t)
{
	if (t == connectTrigger) state->setValueWithData(CONNECTING);
	if (t == takeOffTrigger) state->setValueWithData(TAKEOFF);
	if (t == landTrigger) state->setValueWithData(LANDING);
	if (t == calibrateTrigger) state->setValueWithData(CALIBRATING);
	if (t == rebootTrigger) state->setValueWithData(BOOTING);
	if (t == unlockTrigger) if(state->getValueDataAsEnum<DroneState>() == READY) state->setValueWithData(FLYING);
}


//THREADED
void Drone2::run()
{
	while (!threadShouldExit())
	{
		DroneState newState = state->getValueDataAsEnum<DroneState>();
		if (lastState != newState) setState(newState);

		DroneState s = state->getValueDataAsEnum<DroneState>();

		switch (s)
		{
		case DISCONNECTED:
			break;

		case CONNECTING:
			if(cf != nullptr) cf->sendPing();
			break;

		case CALIBRATING:
			processCalibration();
			break;

		case READY:
			cf->sendPing();
			break;

		case TAKEOFF:
			updateTakeOff();
			break;

		case FLYING:
			updateFlyingPosition();
			break;

		case LANDING:
			cf->sendPing();
			break;

		case ERROR:
			break;
		}

		if(s != DISCONNECTED) checkConnection();

		//Update color
		if (cf != nullptr)
		{
			if (prevHeadLight != headlight->boolValue())
			{
				prevHeadLight = headlight->boolValue();
				setParam("ring", "headlightEnable", headlight->boolValue());
			}

			if (prevLightMode != (int)lightMode->getValueData())
			{
				prevLightMode = (int)lightMode->getValueData();
				setParam("ring", "effect", (int)lightMode->getValueData());
			}

			Colour c = color->getColor();
			if (prevColor != c)
			{
				prevColor = c;
				setParam("ring", "solidRed", c.getRed());
				setParam("ring", "solidGreen", c.getGreen());
				setParam("ring", "solidBlue", c.getBlue());
			}
		}

		
		sleep(50);
	}


	//Exit
	NLOG(niceName, "Clean");
	dataLogBlock = nullptr;
	feedbackBlock = nullptr;
	cf = nullptr;

	state->setValueWithData(DISCONNECTED);
}

void Drone2::setState(DroneState s)
{
	if(lastState == s) return;
	
	if (s != DISCONNECTED && s != CONNECTING && cf == nullptr)
	{
		state->setValueWithData(DISCONNECTED);
		return;
	}

	lastState = s;

	switch (s)
	{
	case DISCONNECTED:
		break;

	case CONNECTING:
		connect();
		break;

	case CALIBRATING:
		lightMode->setValueWithKey("Battery status");
		calibrate();
		break;

	case READY:
		color->setColor(Colours::black); 
		lightMode->setValueWithKey("Solid color");
		break;

	case TAKEOFF:
		setParam("flightmode", "posSet", 0);
		takeOff();
		break;

	case FLYING:
		setParam("flightmode", "posSet", 1); // set absolute
		break;

	case LANDING:
		land();
		break;

	case ERROR:
		break;

	case BOOTING:
		cf->reboot();
		sleep(2000);
		state->setValueWithData(CONNECTING);
		break;
	}


	takeOffTrigger->setEnabled(s == READY);
	landTrigger->setEnabled(s == FLYING);
	connectTrigger->setEnabled(s != FLYING);
	//calibrateTrigger->setEnabled(s != FLYING);
	rebootTrigger->setEnabled(s != FLYING);

}

void Drone2::connect()
{
	NLOG(niceName, "Connecting...");

	
	try
	{

	dataLogBlock = nullptr;
	feedbackBlock = nullptr;

	cf = nullptr; //delete previous
	cf = new Crazyflie(targetRadio->intValue(), channel->intValue(), speed->getValueDataAsEnum<Crazyradio::Datarate>(), address->stringValue());

	sleep(100);
	for (int i = 0; i < 10; i++) cf->sendSetpoint(0, 0, 0, 0); // disable thrust lock, put in autoArm param ?

	cf->requestParamToc();
	
	
	bool selfTest = cf->requestParamValue<uint8>(cf->getParamTocEntry("system", "selftestPassed")->id);
	if (!selfTest)
	{
		NLOGERROR(niceName, "Self test failed");
		state->setValueWithData(ERROR);
	}

	
	//Set the callbacks and logs
	std::function<void(const char *)> consoleF = std::bind(&Drone2::consoleCallback, this, std::placeholders::_1);
	cf->setConsoleCallback(consoleF);

	std::function<void(const crtpPlatformRSSIAck *)> ackF = std::bind(&Drone2::emptyAckCallback, this, std::placeholders::_1);
	cf->setEmptyAckCallback(ackF);

	std::function<void(float)> linkF = std::bind(&Drone2::linkQualityCallback, this, std::placeholders::_1);
	cf->setLinkQualityCallback(linkF);

	cf->logReset();
	cf->requestLogToc(); 

	std::function<void(uint32_t, dataLog *)> cb = std::bind(&Drone2::dataLogCallback, this, std::placeholders::_1, std::placeholders::_2);
	dataLogBlock = new LogBlock<dataLog>(cf, { { "pm","vbat" },{ "pm","state" },{ "kalman","stateX" },{ "kalman","stateY" },{ "kalman","stateZ" } }, cb);
	dataLogBlock->start(10); // 50ms - 20fps

	std::function<void(uint32_t, feedbackLog *)> fcb = std::bind(&Drone2::feedbackLogCallback, this, std::placeholders::_1, std::placeholders::_2);
	feedbackBlock = new LogBlock<feedbackLog>(cf, { { "stabilizer","pitch" },{ "stabilizer","yaw" },{ "stabilizer","roll" } }, fcb);
	feedbackBlock->start(10); // 50ms - 20fps


	NLOG(niceName,"Connected");
	
	state->setValueWithData(CALIBRATING);

	} catch (std::runtime_error &e)
	{
		NLOGERROR(niceName, "Error on drone setup : " << e.what());
	} catch (std::exception &e)
	{
		NLOGERROR(niceName, "Exception on drone setup : " << e.what());
	}
}

void Drone2::checkConnection()
{
	uint32 t = Time::getMillisecondCounter();
	if (lastAckTime > 0 && t > lastAckTime + ackTimeout)
	{
		NLOGWARNING(niceName, "Lost connection");
		state->setValueWithData(DISCONNECTED);
	}
}

void Drone2::calibrate()
{
	NLOG(niceName, "Calibrate");
	
	targetPosition->setVector(0, 0, 0);
	realPosition->setVector(0, 0, 0);
	lastRealPos = Vector3D<float>(0, 0, 0);

	setParam("kalman", "resetEstimation", 1);
	sleep(2);
	setParam("kalman", "resetEstimation", 0);
}

void Drone2::processCalibration()
{
	cf->sendSetpoint(0, 0, 0, 0);

	Vector3D<float> rp = realPosition->getVector();
	if (rp.x == 0 && rp.y == 0 && rp.z == 0) return;

	float dist = (rp - lastRealPos).length();
	if (dist < .2f)
	{
		float t = Time::getMillisecondCounter() / 1000.0f;
		if (timeAtStartConverge == 0) timeAtStartConverge = t;
		else
		{
			if (t > timeAtStartConverge + minConvergeTime)
			{
				NLOG(niceName, "Calibrated");
				state->setValueWithData(READY);
			}
		}
	} else
	{
		timeAtStartConverge = 0;
	}

	lastRealPos = rp;
}

void Drone2::takeOff()
{
	if (cf == nullptr) return;
	timeAtStartTakeOff = Time::getMillisecondCounter() / 1000.0f;
	cf->sendSetpoint(0, 0, 0, 0);
	
	//targetPosition->setVector(targetPosition->x, DroneManager::getInstance()->launchForce->floatValue(), targetPosition->y);
	//state->setValueWithData(FLYING);
}

void Drone2::updateTakeOff()
{
	if (cf == nullptr) return;

	float t = Time::getMillisecondCounter() / 1000.0f;
	float relTime = jmin<float>((t - timeAtStartTakeOff) / DroneManager::getInstance()->launchTime->floatValue(), 1);

	float val = DroneManager::getInstance()->launchCurve.getValueForPosition(relTime);
	float vel = jmap(val, DroneManager::getInstance()->launchMinForce->floatValue(), DroneManager::getInstance()->launchForce->floatValue());
	uint16_t thrust = (uint16_t)(vel * 10000);

	cf->sendSetpoint(0, 0, 0, thrust);
	targetPosition->setVector(realPosition->x,realPosition->y+.3f,realPosition->z);

	if (relTime >= 1) state->setValueWithData(FLYING);
}

void Drone2::updateFlyingPosition()
{
	if (cf == nullptr) return;
	//cf->sendPositionSetpoint(targetPosition->x, targetPosition->z, targetPosition->y, 0); // invert z and y
	cf->sendSetpoint(targetPosition->z, -targetPosition->x, 0, (uint16)(targetPosition->y * 1000));
}

void Drone2::land()
{
	timeAtStartLanding = Time::getMillisecondCounter() / 1000.0f;
}

void Drone2::checkBattery()
{
	if (state->getValueDataAsEnum<DroneState>() != FLYING) return; //only check when flying

	if (voltage->floatValue() > 0) //ensure voltage has been set and drone is not charging
	{
		if (!voltage->boolValue())
		{
			bool isNowLowBattery = voltage->floatValue() <= DroneManager::getInstance()->flyingLowBatteryThreshold->floatValue(); //drone is flying, min battery is 2.7V

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
	} else
	{
		lowBattery->setValue(false);
	}
}


//Drone Helpers
template <class T>
bool Drone2::setParam(String group, String paramID, T value)
{
	try
	{
		const Crazyflie::ParamTocEntry * pte = cf->getParamTocEntry(group.toStdString(), paramID.toStdString());
		if (pte == nullptr) return false;
		//NLOG(niceName, "Set param : " << group << "." << paramID << " : " << (int)value);
		cf->setParam(pte->id, value);
	} catch (std::runtime_error &e)
	{
		NLOG(address->stringValue(), "Error setting param " << group << "." << paramID << " : " << e.what());
		return false;
	}

	return true;
}




//CALLBACKS
void Drone2::consoleCallback(const char * c)
{
	lastAckTime = Time::getApproximateMillisecondCounter();

	consoleBuffer += String(c);

	if (consoleBuffer.endsWith("\n"))
	{
		if (consoleBuffer.toLowerCase().contains("error") || consoleBuffer.toLowerCase().contains("fail")) NLOGERROR(address->stringValue(), consoleBuffer);
		else NLOG(address->stringValue(), consoleBuffer);

		consoleBuffer.clear();
	}
}

void Drone2::emptyAckCallback(const crtpPlatformRSSIAck * a)
{
	lastAckTime = Time::getApproximateMillisecondCounter();
	//inTrigger->trigger();
}

void Drone2::linkQualityCallback(float val)
{
	lastAckTime = Time::getApproximateMillisecondCounter();
	linkQuality->setValue(val);
}

void Drone2::dataLogCallback(uint32_t, dataLog * data)
{
	lastAckTime = Time::getApproximateMillisecondCounter();
	voltage->setValue(data->battery);
	realPosition->setVector(data->x, data->z, data->y); //z is height for the drones, height is y for me.
	//charging->setValue(data->charging == 1);
}

void Drone2::feedbackLogCallback(uint32_t, feedbackLog * data)
{
	lastAckTime = Time::getApproximateMillisecondCounter();
	//orientation->setVector(data->pitch, data->yaw, data->roll);
}