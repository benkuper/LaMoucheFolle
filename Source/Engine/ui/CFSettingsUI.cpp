/*
  ==============================================================================

	CFSettingsUI.cpp
	Created: 13 Jun 2018 7:33:35pm
	Author:  Ben

  ==============================================================================
*/


#include "CFSettingsUI.h"


PhysicsCCUI::PhysicsCCUI(PhysicsCC * cc, bool isRoot) :
	GenericControllableContainerEditor(cc, isRoot),
	pcc(cc)
{
}

PhysicsCCUI::~PhysicsCCUI()
{

}

void PhysicsCCUI::paintOverChildren(Graphics & g)
{
	GenericControllableContainerEditor::paintOverChildren(g);

	if (simRect.getWidth() == 0 || simRect.getHeight() == 0) return;

	g.setColour(BG_COLOR);
	g.fillRect(simRect);

	float maxTime = pcc->simTime->floatValue(); //simulation sur 5seconds
	g.setColour(BG_COLOR.brighter(.2f));

	for (int i = 1; i < maxTime; i++)
	{
		float rel = i * 1.0 / maxTime;
		Point<int> tp = simRect.getRelativePoint(rel, 0.0f);
		Point<int> tp2 = simRect.getRelativePoint(rel, 1.0f);
		g.drawLine(tp.x, tp.y, tp2.x, tp2.y);
	}

	for (int i = ceil(minVal); i < floor(maxVal) && i < minVal+10; i++)
	{
		float ty = jmap<float>(i, minVal, maxVal, simRect.getBottom(), simRect.getY());
		g.drawLine(simRect.getX(), ty, simRect.getRight(), ty);
	}



	g.setColour(GREEN_COLOR.withAlpha(.7f));
	float j0 = jmap<float>(0, minJerk, maxJerk, simRect.getBottom(), simRect.getY());
	g.drawLine(simRect.getX(), j0, simRect.getRight(), j0);
	g.strokePath(jerkPath, PathStrokeType(1));

	g.setColour(BLUE_COLOR.withAlpha(.7f));
	float a0 = jmap<float>(0, minAcc, maxAcc, simRect.getBottom(), simRect.getY());
	g.drawLine(simRect.getX(), a0, simRect.getRight(), a0);
	g.strokePath(accPath, PathStrokeType(1));
	
	g.setColour(RED_COLOR.withAlpha(.7f));
	float s0 = jmap<float>(0, minSpeed, maxSpeed, simRect.getBottom(), simRect.getY());
	g.drawLine(simRect.getX(), s0, simRect.getRight(), s0);
	g.strokePath(speedPath, PathStrokeType(1));

	g.setColour(Colours::white.withAlpha(.8f));
	g.strokePath(simPath, PathStrokeType(2));
}

void PhysicsCCUI::resizedInternalContent(Rectangle<int>& r)
{
	GenericControllableContainerEditor::resizedInternalContent(r);
	r.translate(0, 2);

	simRect = r.withHeight(100);
	r.setY(simRect.getBottom() + 2);

	generateSim();
	repaint();
}

void PhysicsCCUI::generateSim()
{
	minVal = 0;
	maxVal = .001f;
	minSpeed = 0;
	maxSpeed = .001f;
	minAcc = 0;
	maxAcc = .001f;
	minJerk = 0;
	maxJerk = .001f;

	positions.clear();
	speeds.clear();
	accelerations.clear();
	jerks.clear();

	switch (pcc->mode->getValueDataAsEnum<PhysicsCC::ControlMode>())
	{
	case PhysicsCC::DIRECT:
		generateDirect();
		break;

	case PhysicsCC::SPRING:
		generateSpring();
		break;

	case PhysicsCC::JERK:
		generateJerk();
		break;
	}

	simPath.clear();
	simPath.startNewSubPath(simRect.getX(), simRect.getBottom());

	speedPath.clear();
	speedPath.startNewSubPath(simRect.getX(), simRect.getBottom());

	accPath.clear();
	accPath.startNewSubPath(simRect.getX(), simRect.getBottom());

	jerkPath.clear();
	jerkPath.startNewSubPath(simRect.getX(), simRect.getBottom());


	int numValues = positions.size();
	for (int i = 0; i < numValues; i++)
	{
		float rel = i * 1.0f / numValues;

		float tPos = jmap<float>(positions[i], minVal, maxVal, 0, 1);
		float sPos = jmap<float>(speeds[i],minSpeed,maxSpeed,0,1);
		float aPos = jmap<float>(accelerations[i], minAcc, maxAcc, 0, 1);
		float jPos = jmap<float>(jerks[i],minJerk, maxJerk, 0,1);

		float tx = simRect.getX() + rel * simRect.getWidth();
		float tpy = simRect.getBottom() - tPos * simRect.getHeight();
		float tsy = simRect.getBottom() - sPos * simRect.getHeight();
		float tay = simRect.getBottom() - aPos * simRect.getHeight();
		float tjy = simRect.getBottom() - jPos * simRect.getHeight();

		if (!std::isnan(tPos) && !std::isinf(tPos))
		{
			if (i == 0) simPath.startNewSubPath(tx, tpy);
			else simPath.lineTo(tx, tpy);
		}

		if (!std::isnan(sPos) && !std::isinf(sPos))
		{
			if (i == 0) speedPath.startNewSubPath(tx, tsy);
			else speedPath.lineTo(tx, tsy);
		}

		if (!std::isnan(aPos) && !std::isinf(aPos))
		{
			if (i == 0) accPath.startNewSubPath(tx, tay);
			else accPath.lineTo(tx, tay);
		}

		if (!std::isnan(jPos) && !std::isinf(jPos))
		{
			if (i == 0) jerkPath.startNewSubPath(tx, tjy);
			else jerkPath.lineTo(tx, tjy);
		}
	}
}

void PhysicsCCUI::generateDirect()
{
	float tw = (float)getWidth();
	float maxTime = pcc->simTime->floatValue(); //simulation sur 5seconds
	float lastTime = 0;

	float lastPos = 0;
	float lastSpeed = 0; 
	
	const int steps = 3;
	for (int i = 0; i < tw; i += steps)
	{
		float rel = (i * 1.0f / tw);
		float targetPos = pcc->testMotion.getValueForPosition(rel);
		float curTime = rel * maxTime;
		float deltaTime = curTime - lastTime;


		float p = targetPos;
		float s = (targetPos - lastPos)*deltaTime;
		float a = (s - lastSpeed)*deltaTime;

		lastSpeed = s;
		lastPos = p;

		positions.add(p);
		speeds.add(s);
		accelerations.add(a);
		jerks.add(0);


		minVal = jmin(minVal, p);
		maxVal = jmax(maxVal, p);
		minSpeed = jmin(minSpeed, s);
		maxSpeed = jmax(maxSpeed, s);
		minAcc = jmin(minAcc, a);
		maxAcc = jmax(maxAcc, a);

		lastTime = curTime;
	}
}

void PhysicsCCUI::generateSpring()
{
	float tw = (float)getWidth();
	float maxTime = pcc->simTime->floatValue(); //simulation sur 5seconds

	float lastTime = 0; 
	
	float curPos = 0;
	
	float lastPos = 0;
	float lastSpeed = 0;
	float acc = 0;

	float force = pcc->forceFactor->floatValue();
	float frot = pcc->frotFactor->floatValue();

	const int steps = 3;

	for (int i = 0; i < tw; i += steps)
	{
		float rel = (i * 1.0f / tw);

		float targetPos = pcc->testMotion.getValueForPosition(rel);

		float curTime = rel * maxTime;
		float deltaTime = curTime - lastTime;
		
		//Spring
		acc = (targetPos - lastPos) * force - lastSpeed * frot; //frottement - general


		float curSpeed = lastSpeed + acc * deltaTime; // speed calculation - general
		curPos = lastPos + curSpeed * deltaTime; // pos calculation - general

		lastPos = curPos;
		lastSpeed = curSpeed;
		lastTime = curTime;

		positions.add(curPos);
		speeds.add(curSpeed);
		accelerations.add(acc);
		jerks.add(0);

		minVal = jmin(minVal, curPos);
		maxVal = jmax(maxVal, curPos);
		minSpeed = jmin(minSpeed, curSpeed);
		maxSpeed = jmax(maxSpeed, curSpeed);
		minAcc = jmin(minAcc, acc);
		maxAcc = jmax(maxAcc, acc);
	}
}

void PhysicsCCUI::generateJerk()
{

	float jerkMax = pcc->maxJerk->floatValue();
	//float jerkFactor = pcc->jerkFactor->floatValue();
	float accMax = pcc->maxAcceleration->floatValue();
	float sMax = pcc->maxSpeed->floatValue();

	float tw = (float)getWidth();
	float maxTime = pcc->simTime->floatValue(); //simulation sur 5seconds
	float lastTime = 0;

	float lastPos = 0;
	float lastSpeed = 0;
	float lastAcceleration = 0;

	const int steps = 3;
	for (int i = 0; i < tw; i += steps)
	{
		float rel = (i * 1.0f / tw);
		float targetPos = pcc->testMotion.getValueForPosition(rel);
		float curTime = rel * maxTime;
		float deltaTime = curTime - lastTime;

		float diff = targetPos - lastPos;

		float j = jlimit(-jerkMax, jerkMax, diff>1?jerkMax : -jerkMax);
		float a = jlimit(-accMax, accMax, lastAcceleration + j * deltaTime);
		float s = jlimit(-sMax, sMax, lastSpeed + a * deltaTime);
		float p = lastPos + s * deltaTime;

		lastAcceleration = a;
		lastSpeed = s;
		lastPos = p;
		lastTime = curTime;


		positions.add(p);
		speeds.add(s);
		accelerations.add(a);
		jerks.add(j);

		minVal = jmin(minVal, p);
		maxVal = jmax(maxVal, p);
		minSpeed = jmin(minSpeed, s);
		maxSpeed = jmax(maxSpeed, s);
		minAcc = jmin(minAcc, a);
		maxAcc = jmax(maxAcc, a);
		minJerk = jmin(minJerk, j);
		maxJerk = jmax(maxJerk, j);

	}
}

void PhysicsCCUI::controllableFeedbackUpdate(Controllable * c)
{
	generateSim();
	repaint();
}