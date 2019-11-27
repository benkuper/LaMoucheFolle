/*
  ==============================================================================

    CFSettingsUI.h
    Created: 13 Jun 2018 7:33:35pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "../CFSettings.h"


class PhysicsCCUI :
	public GenericControllableContainerEditor
{
public:
	PhysicsCCUI(PhysicsCC * cc, bool isRoot = false);
	~PhysicsCCUI();


	PhysicsCC * pcc;

	Rectangle<int> simRect;
	Path simPath;
	Path speedPath;
	Path accPath;
	Path jerkPath;

	float minVal;
	float maxVal;
	float minSpeed;
	float maxSpeed;
	float minAcc;
	float maxAcc;
	float minJerk;
	float maxJerk;

	Array<float> positions;
	Array<float> speeds;
	Array<float> accelerations;
	Array<float> jerks;

	void paintOverChildren(Graphics &g) override;

	void resizedInternalContent(Rectangle<int> &r) override;
	void generateSim();

	void generateDirect();
	void generateSpring();
	void generateJerk();

	void controllableFeedbackUpdate(Controllable * c) override;
};
