/*
  ==============================================================================

    CFSettings.h
    Created: 12 Jun 2018 10:19:04pm
    Author:  Ben

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class PhysicsCC :
	public ControllableContainer
{
public:
	PhysicsCC();
	~PhysicsCC();

	struct PhysicalState
	{
		PhysicalState() {}
		PhysicalState(Vector3D<float> position, Vector3D<float> speed, Vector3D<float> acceleration) :
			position(position), speed(speed), acceleration(acceleration) {}

		Vector3D<float> position;
		Vector3D<float> speed;
		Vector3D<float> acceleration;
		Vector3D<float> jerk;
	};


	enum ControlMode { DIRECT, SPRING, JERK };
	EnumParameter * mode;
	FloatParameter * forceFactor;
	FloatParameter * frotFactor;

	FloatParameter * maxJerk;
	FloatParameter * jerkFactor;
	FloatParameter * maxAcceleration;
	FloatParameter * maxSpeed;

	//Viz
	FloatParameter * simTime;
	Automation testMotion;

	void onContainerParameterChanged(Parameter * p) override;

	PhysicalState processPhysics(float deltaTime, const PhysicalState &currentState, const PhysicalState &desiredState) const;

	InspectableEditor * getEditor(bool isRoot) override;
};

class CFSettings : public ControllableContainer
{
public:
	CFSettings();
	~CFSettings();

	juce_DeclareSingleton(CFSettings, true)

	ControllableContainer setupCC;

	BoolParameter * autoConnect;
	BoolParameter * analyzeAfterConnect;
	BoolParameter * calibAfterConnect;
	BoolParameter * calibAfterAnalyze;
	Point3DParameter * lpsBoxSize;
	FloatParameter * lpsGroundHeight;

	FloatParameter * minBattery;
	FloatParameter * lowBatteryTime;

	EnumParameter * lpsMode;

	ControllableContainer lighthouseCC;

	Point3DParameter* bs1Origin;	
	Array<Point3DParameter*> bs1MatRows;
	Point3DParameter* bs2Origin;
	Array<Point3DParameter*> bs2MatRows;

	ControllableContainer flightCC;
	FloatParameter * takeOffHeight;
	BoolParameter * useThrustCommand;
	FloatParameter * takeOffTime;
	FloatParameter * takeOffMaxSpeed;
	FloatParameter * takeOffMinSpeed;
	Automation takeOffCurve;
	BoolParameter * disableYawCommand;

	PhysicsCC physicsCC;

	ControllableContainer miscCC;
	
	EnumParameter * units;
	EnumParameter * leftRightAxis;
	EnumParameter * downUpAxis;
	EnumParameter * frontBackAxis;
	BoolParameter * invertLeftRight;
	BoolParameter * invertFrontBack;

	
	static Vector3D<float> toDroneVector(Vector3D<float> lmfVector, bool convertAxis = true, bool convertUnit = true);
	static Vector3D<float> toLMFVector(Vector3D<float> droneVector, bool convertAxis = true, bool convertUnit = true);
};


