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
	public EnablingControllableContainer
{
public:
	PhysicsCC();
	~PhysicsCC();

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

	ControllableContainer flightCC;
	FloatParameter * takeOffTime;
	FloatParameter * takeOffMaxSpeed;
	Automation takeOffCurve;

	PhysicsCC physicsCC;

	ControllableContainer miscCC;
	BoolParameter * zIsVertical;

};


