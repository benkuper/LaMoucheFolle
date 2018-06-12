/*
  ==============================================================================

    DroneGridUI.cpp
    Created: 12 Jun 2018 4:00:58pm
    Author:  Ben

  ==============================================================================
*/

#include "DroneGridUI.h"

DroneGridUI::DroneGridUI(Drone * drone) :
	BaseItemMinimalUI(drone),
	RadialMenuTarget(this)
{
	updateUI();
}

DroneGridUI::~DroneGridUI()
{
}

void DroneGridUI::paint(Graphics & g)
{
	if (droneImage.getWidth() > 0) g.drawImage(droneImage, getLocalBounds().toFloat());
	if (overlayImage.getWidth() > 0) g.drawImage(overlayImage, getLocalBounds().reduced(20).toFloat());

	//Progress
	float progress = 0;
	if (progress > 0)
	{
		Path p;
		p.addArc(0, 0, getWidth(), getHeight(), 0, float_Pi * 2 * progress, true);

		g.setColour(Colours::yellow);
		g.strokePath(p, PathStrokeType(4));
	}
}

void DroneGridUI::updateUI()
{
	droneImage = VizImages::getDroneStateImage(item);
	overlayImage = VizImages::getDroneOverlayImage(item);
	repaint();
}

void DroneGridUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	if (c == item->state) updateUI();
}



Image VizImages::getDroneStateImage(Drone * d)
{
	Drone::DroneState s = d->state->getValueDataAsEnum<Drone::DroneState>();
	switch (s)
	{

	case Drone::POWERED_OFF:
		return ImageCache::getFromMemory(BinaryData::drone_poweroff_png, BinaryData::drone_poweroff_pngSize);

	case Drone::DISCONNECTED:
		return ImageCache::getFromMemory(BinaryData::drone_poweron_png, BinaryData::drone_poweron_pngSize);

	case Drone::CONNECTING:
	case Drone::CALIBRATING:
	case Drone::ANALYSIS:
		return ImageCache::getFromMemory(BinaryData::drone_connecting_png, BinaryData::drone_connecting_pngSize);

	case Drone::READY:
		return ImageCache::getFromMemory(BinaryData::drone_ok_png, BinaryData::drone_ok_pngSize);

	case Drone::WARNING:
		return ImageCache::getFromMemory(BinaryData::warning_png, BinaryData::warning_pngSize);

	case Drone::ERROR:
		return ImageCache::getFromMemory(BinaryData::drone_error_png, BinaryData::drone_error_pngSize);
	}

	return Image();
}

Image VizImages::getDroneOverlayImage(Drone * d)
{
	Drone::DroneState s = d->state->getValueDataAsEnum<Drone::DroneState>();
	switch (s)
	{
	case Drone::CALIBRATING:
		return ImageCache::getFromMemory(BinaryData::calibrating_png, BinaryData::calibrating_pngSize);

	case Drone::ANALYSIS:
		return ImageCache::getFromMemory(BinaryData::health_analysis_png, BinaryData::health_analysis_pngSize);


	case Drone::WARNING:
		if (d->batteryProblem->boolValue()) return ImageCache::getFromMemory(BinaryData::battery_problem_png, BinaryData::battery_problem_pngSize);
		if (d->selfTestProblem->boolValue() || !d->allDecksAreConnected()) return ImageCache::getFromMemory(BinaryData::config_problem_png, BinaryData::config_problem_pngSize);
		break;

	case Drone::READY:
		if (d->isLaunching) return ImageCache::getFromMemory(BinaryData::startup_png, BinaryData::startup_pngSize);
		if (d->isLanding) return ImageCache::getFromMemory(BinaryData::parachute_png, BinaryData::parachute_pngSize);
		break;
	}

	return Image();
}

