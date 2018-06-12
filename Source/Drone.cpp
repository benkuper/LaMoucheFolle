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
	radioCC("Radio"),
	controlsCC("Controls"),
	statusCC("Status"),
	decksCC("Decks"),
	wingsCC("Wings"),
	flightCC("Flight"),
	lightingCC("Lights"),
	timeAtStartTakeOff(0),
	timeAtStartLanding(0),
	timeAtStartConverge(0),
	isLaunching(false),
	isLanding(false),
	calibrationProgress(0),
	analysisProgress(0)
{
	addChildControllableContainer(&radioCC);
	autoRadio = radioCC.addBoolParameter("Auto radio", "If enabled, radio will automatically be set depending on the channel : Channel 0-> 9 = radio 0, Channel 10->19 = radio 1...", false);
	autoChannel = radioCC.addBoolParameter("Auto channel", "If enabled, the last byte of the address will be used to automatically set the drone channel : 00->0F = Channel 0, 10->1F = Channel 10, 20->2F = Channel 20...",false);
	targetRadio = radioCC.addIntParameter("Radio", "The USB Crazyradio to use to connect to the drone", 0, 0, 100);
	channel = radioCC.addIntParameter("Channel", "The Channel set up in the drone", 0, 0, 125);
	baudRate = radioCC.addEnumParameter("Baud Rate", "The baudrate to use to connect to the drone");
	baudRate->addOption("2M", Crazyradio::Datarate_2MPS)->addOption("250K", Crazyradio::Datarate_250KPS)->addOption("1M", Crazyradio::Datarate_1MPS);
	address = radioCC.addStringParameter("Address", "Address of the drone, without the 0x", "E7E7E7E7E7");

	addChildControllableContainer(&controlsCC);
	connectTrigger = controlsCC.addTrigger("Connect", "Connect to the drone");
	calibrateTrigger = controlsCC.addTrigger("Calibrate", "Reset the kalman filter");
	takeOffTrigger = controlsCC.addTrigger("Take off", "Make the drone take off");
	landTrigger = controlsCC.addTrigger("Land", "Land the drone");
	rebootTrigger = controlsCC.addTrigger("Reboot", "Reboot the drone");

	addChildControllableContainer(&flightCC);
	realPosition = flightCC.addPoint3DParameter("Real Position", "Real Position feedback from the drone");
	targetPosition = flightCC.addPoint3DParameter("Target", "Target Position for the drone");
	yaw = flightCC.addFloatParameter("Yaw", "The target horizontal rotation of the drone, 0 is aligned to X+", 0, 0, 360);

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
	state = statusCC.addEnumParameter("State", "State of the drone");
	state->addOption("Powered Off", POWERED_OFF)->addOption("Disconnected", DISCONNECTED)->addOption("Connecting", CONNECTING)
		->addOption("Calibrating", CALIBRATING)->addOption("Analysis", ANALYSIS)
		->addOption("Warning", WARNING)->addOption("Ready", READY)->addOption("Error", ERROR);
	linkQuality = statusCC.addFloatParameter("Link Quality", "Quality of radio communication with the drone", 0, 0, 1);

	batteryLevel = statusCC.addFloatParameter("BatteryLevel", "Voltage of the drone. /!\\ When flying, there is a massive voltage drop !", 0, 0, 100);
	charging = statusCC.addBoolParameter("Charging", "Is the drone charging", false);
	lowBattery = statusCC.addBoolParameter("Low Battery", "Low battery", false);

	selfTestProblem = statusCC.addBoolParameter("SelfTest Problem", "There was a problem during the selfTest", false);
	batteryProblem = statusCC.addBoolParameter("Battery Problem", "If on, you should replace this battery !", false);
	for (auto &c : statusCC.controllables) c->setControllableFeedbackOnly(true);


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
	waitForThreadToExit(1000);
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
		landTrigger->setEnabled(curState == READY);
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
			state->setValue(DISCONNECTED);
		}
	}

	if (c == headlight) setParam("ring", "headlightEnable", headlight->boolValue());
	else if (c == lightMode) setParam("ring", "effect", (int)lightMode->getValueData());
	else if (c == color) setParam("ring", "fadeColor", color->getColor().getARGB());
	else if (c == fadeTime) setParam("ring", "fadeTime", fadeTime->floatValue());
	else if (c == stealthMode) setParam("platform", "stealthMode", stealthMode->intValue());

	if (c == address || c == autoRadio || c == autoChannel)
	{
		int64 lastByte = address->stringValue().getHexValue64() & 0xFF;
		int rIndex = floor(lastByte / 16);
		if (autoRadio->boolValue()) targetRadio->setValue(rIndex);
		if (autoChannel->boolValue()) channel->setValue(rIndex * 10);
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

void Drone::emptyAckCallback(const crtpPlatformRSSIAck * a)
{
	//ack
}

void Drone::linkQualityCallback(float val)
{
	linkQuality->setValue(0);
}

void Drone::batteryLogCallback(uint32_t, BatteryLog * data)
{
	batteryLevel->setValue(data->battery);
	lowBattery->setValue(data->lowBattery);
	charging->setValue(data->charging > 0);
}

void Drone::posLogCallback(uint32_t, PosLog * data)
{
	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	realPosition->setVector(data->x, zIsVertical ? data->y : data->z, zIsVertical ? data->z : data->y);
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

void Drone::run()
{
	try
	{
		
		while (!threadShouldExit())
		{
			DroneState s = state->getValueDataAsEnum<DroneState>();

			switch (s)
			{
			case POWERED_OFF:
				ping();
				break;

			case DISCONNECTED:
				break;

			case CONNECTING:
				break;

			case CALIBRATING:
				processCalibration();
				break;

			case ANALYSIS:
				break;

			case READY:
				updateFlyingPosition();
				break;

			case WARNING:
				break;

			case ERROR:
				break;
			}


			sleep(50);
		}

		//Exit
		batteryLogBlock = nullptr;
		posLogBlock = nullptr;
		cf = nullptr;

		state->setValueWithData(DISCONNECTED);
	} catch (std::runtime_error &e)
	{
		NLOG(niceName,"Runtime error " << e.what());
	} catch (std::exception &e)
	{
		NLOG(niceName, "Exception " << e.what());
	}

}

void Drone::stateUpdated()
{
	DroneState s = state->getValueDataAsEnum<DroneState>();
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

	case ANALYSIS:
		break;

	case READY:
		color->setColor(Colours::black);
		lightMode->setValueWithKey("Fade Color");
		break;

	case WARNING:
		break;

	case ERROR:
		break;

	}

	
}

void Drone::ping()
{
	if (cf == nullptr) return;
	cf->sendPing();
}

void Drone::connect()
{
	NLOG(niceName, "Connecting...");

	cf = nullptr; //delete previous
	cf = new Crazyflie(targetRadio->intValue(), channel->intValue(), baudRate->getValueDataAsEnum<Crazyradio::Datarate>(), address->stringValue());

	//Reset all parameters

	lowBattery->setValue(false);
	batteryProblem->setValue(false);
	selfTestProblem->setValue(false);
	batteryLevel->setValue(0);
	charging->setValue(false);
	chargingProgress->setValue(false);
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
		batteryLogBlock = nullptr;
		posLogBlock = nullptr;

		cf->requestParamToc();

		bool selfTest = cf->requestParamValue<uint8>(cf->getParamTocEntry("system", "selftestPassed")->id);

		selfTestProblem->setValue(!selfTest);
		if (!selfTest)
		{
			NLOGERROR(niceName, "Self test failed");
			state->setValueWithData(WARNING);
			return;
		}


		//Set the callbacks and logs
		std::function<void(const char *)> consoleF = std::bind(&Drone::consoleCallback, this, std::placeholders::_1);
		cf->setConsoleCallback(consoleF);

		std::function<void(const crtpPlatformRSSIAck *)> ackF = std::bind(&Drone::emptyAckCallback, this, std::placeholders::_1);
		cf->setEmptyAckCallback(ackF);

		std::function<void(float)> linkF = std::bind(&Drone::linkQualityCallback, this, std::placeholders::_1);
		cf->setLinkQualityCallback(linkF);

		cf->logReset();
		cf->requestLogToc();

		std::function<void(uint32_t, BatteryLog *)> cb = std::bind(&Drone::batteryLogCallback, this, std::placeholders::_1, std::placeholders::_2);
		batteryLogBlock = new LogBlock<BatteryLog>(cf, { {"pm","batteryLevel" }, {"pm","lowBattery"},{ "pm","state" } }, cb);
		batteryLogBlock->start(30);

		std::function<void(uint32_t, PosLog *)> fcb = std::bind(&Drone::posLogCallback, this, std::placeholders::_1, std::placeholders::_2);
		posLogBlock = new LogBlock<PosLog>(cf, { { "kalman","stateX" },{ "kalman","stateY" },{ "kalman","stateZ" } }, fcb);
		posLogBlock->start(20); //20ms = 50fps

		NLOG(niceName, "Connected");

		state->setValueWithData(CALIBRATING);

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
	calibrationProgress = 0;
}

void Drone::processCalibration()
{
}

void Drone::takeOff()
{
}

void Drone::updateTakeOff()
{
}

void Drone::updateFlyingPosition()
{
	bool zIsVertical = CFSettings::getInstance()->zIsVertical->boolValue();
	cf->sendPositionSetpoint(targetPosition->x, zIsVertical ? targetPosition->y : targetPosition->z, zIsVertical ? targetPosition->z : targetPosition->y, yaw->floatValue());
}

void Drone::land()
{

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

	stopThread(100);
	startThread();
}


template<class T>
bool Drone::setParam(String group, String paramID, T value)
{
	if (cf == nullptr) return false;
	try
	{
		SpinLock::ScopedLockType lock(paramLock);

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