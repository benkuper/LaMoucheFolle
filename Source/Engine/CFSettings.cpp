/*
  ==============================================================================

	CFSettings.cpp
	Created: 12 Jun 2018 10:19:04pm
	Author:  Ben

  ==============================================================================
*/

#include "CFSettings.h"
#include "ui/CFSettingsUI.h"
#include "Drone/Drone.h"

juce_ImplementSingleton(CFSettings)

CFSettings::CFSettings() :
	ControllableContainer("Crazyflie settings"),
	lighthouseCC("Lighthouse"),
	flightCC("Flight"),
	conversionCC("Conversion"),
	setupCC("Setup")
{
	addChildControllableContainer(&setupCC);
	//lpsMode = setupCC.addEnumParameter("LPS Mode", "The mode to set the drone at connect");
	//lpsMode->addOption("Do not set",-1)->addOption("Auto", 0)->addOption("TDoA 2", 2)->addOption("TDoA 3", 3);
	autoConnect = setupCC.addBoolParameter("Auto connect", "If checked, detected drones will be automatically connected", false);
	//analyzeAfterConnect = setupCC.addBoolParameter("Analyze after connect", "If checked, a health check will be performed after each connection", false);
	//calibAfterConnect = setupCC.addBoolParameter("Calibrate after connect", "If checked and 'Analyze after connect' is unchecked, a calibration be performed after each connection", true);
	//calibAfterAnalyze = setupCC.addBoolParameter("Calibrate after Analysis", "If checked, a calibration will be trigger after each analysis", true);

	//minBattery = setupCC.addFloatParameter("Min Battery", "Minimum amount after which drone will be in low battery mode", 3.3f, 3, 4.2f);
	//lowBatteryTime = setupCC.addFloatParameter("Low Battery Threshold", "Time to consider low battery after battery level is below the minimum", 4.f, 0, 10);

	//lpsBoxSize = addPoint3DParameter("LPS Box Size", "Size of the box enclosed by the LPS Nodes. Origin is floor center.\n \
	//									Nodes are assumed to be positionned 0 left front ground > 1,2,3 clockwise on ground, 4 on top of 0 > 5,6,7 clockwise up");
	//lpsBoxSize->setVector(4, 3, 4);

	//lpsGroundHeight = addFloatParameter("LPS Ground Height", "Vertical height from ground for the floor anchors. Does not affect box size", 0);


	flightController = addEnumParameter("Flight Controller", "Controller used for flying.");
	for (int i = 0; i < CONTROLLER_MAX; i++) flightController->addOption(controllerNames[i], (FlightController)i);

	copyBSFromClipboard = lighthouseCC.addTrigger("Copy from clipboard","Copy from clipboard");
	
	bs1Origin = lighthouseCC.addPoint3DParameter("BS1 Origin", "BS1");
	for (int i = 0; i < 3; i++) bs1MatRows.add(lighthouseCC.addPoint3DParameter("BS1 Row "+String(i+1), ""));

	bs2Origin = lighthouseCC.addPoint3DParameter("BS2 Origin", "BS2");
	for (int i = 0; i < 3; i++) bs2MatRows.add(lighthouseCC.addPoint3DParameter("BS2 Row " + String(i + 1), ""));

	addChildControllableContainer(&lighthouseCC);
	lighthouseCC.enabled->setValue(false);



	addChildControllableContainer(&flightCC);
	flightCC.saveAndLoadRecursiveData = true;
	takeOffHeight = flightCC.addFloatParameter("TakeOff Height", "Height to take off the drone", .5, 0, 3);
	takeOffTime = flightCC.addFloatParameter("Takeoff Time", "Time to take off, in seconds", 2,0);

	disableYawCommand = flightCC.addBoolParameter("Disable Yaw Commands", "If enabled, the drones won't receive any yaw command, and position command will be set to yaw = 0", false);

	//flightCC.addChildControllableContainer(&physicsCC);

	addChildControllableContainer(&conversionCC);
	units = conversionCC.addEnumParameter("Units", "Cheese eater or Uncle sam ?");
	units->addOption("Cheese Eater", 1)->addOption("Uncle Sam", 3.28f);

	leftRightAxis = conversionCC.addEnumParameter("Left-Right axis", "This decides which variable to use for left to right.");
	leftRightAxis->addOption("X", 0)->addOption("Y", 1)->addOption("Z", 2);
	
	downUpAxis = conversionCC.addEnumParameter("Down-Up axis", "This decides which variable to use for down to up.");
	downUpAxis->addOption("X", 0)->addOption("Y", 1)->addOption("Z", 2);
	downUpAxis->setValueWithKey("Y");

	frontBackAxis = conversionCC.addEnumParameter("Front-Back axis", "This decides which variable to use for left to right.");
	frontBackAxis->addOption("X", 0)->addOption("Y", 1)->addOption("Z", 2);
	frontBackAxis->setValueWithKey("Z");

	addChildControllableContainer(&deckManager);
}

CFSettings::~CFSettings()
{
}

void CFSettings::copyBSMatricesFromClipboard()
{
	String s = SystemClipboard::getTextFromClipboard();
	var data = JSON::parse(s);
	if (!data.isArray())
	{
		LOGWARNING("Data is not an array");
		return;
	}

	if (data.size() < 2)
	{
		LOGWARNING("Data needs at least 2 entries");
		return;
	}

	var o1 = data[0].getProperty("origin",var());
	bs1Origin->setVector(o1[0], o1[1], o1[2]);
	var o2 = data[1].getProperty("origin", var());
	bs2Origin->setVector(o2[0], o2[1], o2[2]);

	var mat1 = data[0].getProperty("mat", var());
	var mat2 = data[1].getProperty("mat", var());
	for (int i = 0; i < 3; i++)
	{
		bs1MatRows[i]->setVector(mat1[i][0], mat1[i][1], mat1[i][2]);
		bs2MatRows[i]->setVector(mat2[i][0], mat2[i][1], mat2[i][2]);
	}
	
}

void CFSettings::controllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	if (c == copyBSFromClipboard)
	{
		copyBSMatricesFromClipboard();
	}
}

var CFSettings::getJSONData()
{
	var data = ControllableContainer::getJSONData();
	data.getDynamicObject()->setProperty("setup", setupCC.getJSONData());
	data.getDynamicObject()->setProperty("lighthouse", lighthouseCC.getJSONData());
	data.getDynamicObject()->setProperty("flight", flightCC.getJSONData());
	data.getDynamicObject()->setProperty("conversion", conversionCC.getJSONData());
	data.getDynamicObject()->setProperty("decks", deckManager.getJSONData());
	return data;
}

void CFSettings::loadJSONDataInternal(var data)
{
	setupCC.loadJSONData(data.getProperty("setup",var()));
	lighthouseCC.loadJSONData(data.getProperty("lighthouse",var()));
	flightCC.loadJSONData(data.getProperty("flight",var()));
	conversionCC.loadJSONData(data.getProperty("conversion", var()));
	deckManager.loadJSONData(data.getProperty("decks", var()));
}

Vector3D<float> CFSettings::toDroneVector(Vector3D<float> lmfVector, bool convertAxis, bool convertUnits)
{
	if (getInstanceWithoutCreating() == nullptr) return Vector3D<float>();
	CFSettings * s = getInstance();


	Vector3D<float> result(lmfVector.x, lmfVector.y, lmfVector.z);
	
	if (convertAxis)
	{
	
		const float lmfValues[3]{ lmfVector.x, lmfVector.y, lmfVector.z };
		result = Vector3D<float>(
			lmfValues[s->leftRightAxis->intValue()],
			lmfValues[s->frontBackAxis->intValue()],
			lmfValues[s->downUpAxis->intValue()]
			);
	};

	if (convertUnits) result /= s->units->floatValue();
	return result;
}

Vector3D<float> CFSettings::toLMFVector(Vector3D<float> droneVector, bool convertAxis, bool convertUnits)
{
	if (getInstanceWithoutCreating() == nullptr) return Vector3D<float>();
	CFSettings * s = getInstance();

	Vector3D<float> result(droneVector.x, droneVector.y, droneVector.z);
	
	if (convertAxis)
	{
		//Drone is X right and Z vertical, so it should be in this order : left-right / front-back / down-up
		float lmfValues[3];
		lmfValues[s->leftRightAxis->intValue()] = droneVector.x;
		lmfValues[s->downUpAxis->intValue()] = droneVector.z;
		lmfValues[s->frontBackAxis->intValue()] = droneVector.y;

		//Drone is X right and Z vertical, so it should be in this order : left-right / front-back / down-up
		result = Vector3D<float>(lmfValues[0], lmfValues[1], lmfValues[2]);
	}

	if (convertUnits) result *= s->units->floatValue();
	return result;
}


PhysicsCC::PhysicsCC() :
	ControllableContainer("Physics"),
	testMotion("Test Motion")
{
	saveAndLoadRecursiveData = true;


	mode = addEnumParameter("Mode", "Control Mode");
	mode->addOption("Direct", DIRECT)->addOption("Spring", SPRING)->addOption("Jerk", JERK);

	forceFactor = addFloatParameter("Flight Speed Factor", "Aggressiveness of the flight", 10, 0, 1000, false);
	frotFactor = addFloatParameter("Frottements", "", 5, 1, 50, false);

	maxSpeed = addFloatParameter("Max Speed", "Max speed in m/s", 1, 0.001f, 200, false);
	maxAcceleration = addFloatParameter("Max Acceleration", "Max speed in m/s/s", 1, 0.001f, 1000, false);
	maxJerk = addFloatParameter("Max Jerk", "Max speed in m/s/s/s", 1, 0.001f, 1000, false);
	jerkFactor = addFloatParameter("Jerk Factor", "multiply dist to point by that in jerk", 1, 1, 1000, false);

	simTime = addFloatParameter("Sim Time", "Time of the simulated curve in seconds", 5, 1, 20);

	addChildControllableContainer(&testMotion);
	testMotion.showUIInEditor = true;
	testMotion.addItem(0, 0);
	testMotion.addItem(.2f, 1);
	testMotion.items[0]->setEasing(Easing::HOLD);
	testMotion.enableSnap->setValue(false);
}

PhysicsCC::~PhysicsCC()
{
}

void PhysicsCC::onContainerParameterChanged(Parameter * p)
{
	if (p == mode)
	{
		ControlMode cm = mode->getValueDataAsEnum<ControlMode>();
		forceFactor->setEnabled(cm == SPRING);
		frotFactor->setEnabled(cm == SPRING);

		maxSpeed->setEnabled(cm == JERK);
		maxAcceleration->setEnabled(cm == JERK);
		maxJerk->setEnabled(cm == JERK);
		jerkFactor->setEnabled(cm == JERK);
	}
}

PhysicsCC::PhysicalState PhysicsCC::processPhysics(float deltaTime, const PhysicalState & currentState, const PhysicalState & desiredState) const
{
	ControlMode m = mode->getValueDataAsEnum<ControlMode>();

	PhysicalState result;
	switch (m)
	{
	case DIRECT:
		return desiredState;
		break;

	case SPRING:
	{
		//Spring
		result.acceleration = (desiredState.position - currentState.position) * forceFactor->floatValue();
		result.acceleration -= Vector3D<float>(currentState.speed) * frotFactor->floatValue(); //frottement - general

		result.speed = currentState.speed + result.acceleration * deltaTime; // speed calculation - general
		result.position = currentState.position + result.speed * deltaTime; // pos calculation - general
	}
	break;

	default:
		//not handle
		break;
	}

	return result;
}

InspectableEditor * PhysicsCC::getEditor(bool isRoot)
{
	return new PhysicsCCUI(this, isRoot);
}
