/*
  ==============================================================================

    DroneGridUI.cpp
    Created: 12 Jun 2018 4:00:58pm
    Author:  Ben

  ==============================================================================
*/

#include "DroneGridUI.h"

DroneGridUI::DroneGridUI(CFDrone * drone) :
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
	CFDrone::DroneState s = item->state->getValueDataAsEnum<CFDrone::DroneState>();
	float progress = 0;
	switch (s)
	{
	case CFDrone::CALIBRATING: progress = item->calibrationProgress->floatValue(); break;
	case CFDrone::ANALYSIS: progress = item->analysisProgress->floatValue(); break;
	}

	if (progress > 0)
	{
		Path p;
		p.addArc(10, 10, getWidth()-20, getHeight()-20, 0, float_Pi * 2 * progress, true);

		g.setColour(Colours::yellow);
		g.strokePath(p, PathStrokeType(4));
	}

	Rectangle<float> tr = getLocalBounds().reduced(20,0).withHeight(14).toFloat();
	g.setColour(BG_COLOR.brighter().withAlpha(.6f));
	g.fillRoundedRectangle(tr, 4);
	g.setColour(BG_COLOR.brighter(.3f).withAlpha(.6f));
	g.drawRoundedRectangle(tr, 4,1);
	g.setColour(TEXT_COLOR);
	g.drawFittedText(item->niceName, tr.toNearestInt(), Justification::centred, 1);
}

void DroneGridUI::updateUI()
{
	droneImage = VizImages::getDroneStateImage(item);
	overlayImage = VizImages::getDroneOverlayImage(item);
	repaint();
}

void DroneGridUI::controllableFeedbackUpdateInternal(Controllable * c)
{
	if (c == item->state || c == item->calibrationProgress || c == item->analysisProgress) updateUI();
}

void DroneGridUI::containerChildAddressChangedAsync(ControllableContainer *)
{
	repaint();
}



Image VizImages::getDroneStateImage(CFDrone * d)
{
	CFDrone::DroneState s = d->state->getValueDataAsEnum<CFDrone::DroneState>();
	switch (s)
	{

	case CFDrone::POWERED_OFF:
		return ImageCache::getFromMemory(BinaryData::drone_poweroff_png, BinaryData::drone_poweroff_pngSize);

	case CFDrone::DISCONNECTED:
		return ImageCache::getFromMemory(BinaryData::drone_poweron_png, BinaryData::drone_poweron_pngSize);

	case CFDrone::CONNECTING:
	case CFDrone::CALIBRATING:
	case CFDrone::ANALYSIS:
		return ImageCache::getFromMemory(BinaryData::drone_connecting_png, BinaryData::drone_connecting_pngSize);

	case CFDrone::READY:
	case CFDrone::TAKING_OFF:
	case CFDrone::FLYING:
	case CFDrone::LANDING:
	{
		if (d->lowBattery->boolValue()) return ImageCache::getFromMemory(BinaryData::drone_lowbattery_png, BinaryData::drone_lowbattery_pngSize);
		else return ImageCache::getFromMemory(BinaryData::drone_ok_png, BinaryData::drone_ok_pngSize);
	}
	break;

	case CFDrone::WARNING: return ImageCache::getFromMemory(BinaryData::drone_warning_png, BinaryData::drone_warning_pngSize);
	
	case CFDrone::ERROR:
		return ImageCache::getFromMemory(BinaryData::drone_error_png, BinaryData::drone_error_pngSize);
	}

	return Image();
}

Image VizImages::getDroneOverlayImage(CFDrone * d)
{
	CFDrone::DroneState s = d->state->getValueDataAsEnum<CFDrone::DroneState>();
	switch (s)
	{
	case CFDrone::CALIBRATING:
		return ImageCache::getFromMemory(BinaryData::calibrating_png, BinaryData::calibrating_pngSize);

	case CFDrone::ANALYSIS:
		return ImageCache::getFromMemory(BinaryData::health_analysis_png, BinaryData::health_analysis_pngSize);

	case CFDrone::WARNING:
		if (d->selfTestProblem->boolValue() /*|| !d->allDecksAreConnected()*/) return ImageCache::getFromMemory(BinaryData::config_problem_png, BinaryData::config_problem_pngSize);
		if (d->batteryProblem->boolValue()) return ImageCache::getFromMemory(BinaryData::battery_problem_png, BinaryData::battery_problem_pngSize);
		if (!d->isCalibrated) return ImageCache::getFromMemory(BinaryData::badcalib_png, BinaryData::badcalib_pngSize);
		if(d->upsideDown->boolValue()) return ImageCache::getFromMemory(BinaryData::upsidedown_png, BinaryData::upsidedown_pngSize);
		break;

	case CFDrone::TAKING_OFF:
		return ImageCache::getFromMemory(BinaryData::startup_png, BinaryData::startup_pngSize);
	
	case CFDrone::FLYING:
		return ImageCache::getFromMemory(BinaryData::flying_png, BinaryData::flying_pngSize);

	case CFDrone::LANDING:
		return ImageCache::getFromMemory(BinaryData::parachute_png, BinaryData::parachute_pngSize);
	}

	return Image();
}

