/*
  ==============================================================================

    DroneUI.cpp
    Created: 25 Nov 2019 3:33:51pm
    Author:  bkupe

  ==============================================================================
*/

#include "DroneUI.h"

DroneUI::DroneUI(Drone* drone) :
	BaseItemUI(drone)
{
}

DroneUI::~DroneUI()
{
}

void DroneUI::paint(Graphics& g)
{
	g.setColour(getColorForState());
	g.fillEllipse(statusRect.toFloat());
}

void DroneUI::resizedInternalHeader(Rectangle<int>& r)
{
	statusRect = r.removeFromRight(r.getHeight()).reduced(1);
}

void DroneUI::controllableFeedbackUpdateInternal(Controllable* c)
{
	BaseItemUI::controllableFeedbackUpdateInternal(c);
	if (c == item->state)
	{
		repaint();
	}
}

Colour DroneUI::getColorForState()
{
	Drone::State s = item->state->getValueDataAsEnum<Drone::State>();
	switch (s)
	{
	case Drone::DISCONNECTED: return NORMAL_COLOR.brighter();
	case Drone::CONNECTING: return BLUE_COLOR.darker();
	case Drone::READY: return GREEN_COLOR.darker();
	case Drone::TAKING_OFF: return Colours::lightpink;
	case Drone::FLYING: return Colours::orange;
	case Drone::LANDING: return Colours::orangered;
	case Drone::WARNING: return YELLOW_COLOR;
	}

	return Colours::black;
}



//


Image VizImages::getDroneStateImage(Drone* d)
{
	Drone::State s = d->state->getValueDataAsEnum<Drone::State>();
	switch (s)
	{

		//case Drone::POWERED_OFF:
		//	
	case Drone::DISCONNECTED:
		return ImageCache::getFromMemory(BinaryData::drone_poweroff_png, BinaryData::drone_poweroff_pngSize);

	case Drone::CONNECTING:
		return ImageCache::getFromMemory(BinaryData::drone_connecting_png, BinaryData::drone_connecting_pngSize);


	case Drone::READY:
	case Drone::TAKING_OFF:
	case Drone::FLYING:
	case Drone::LANDING:
	{
		/*if (d->lowBattery->boolValue()) return ImageCache::getFromMemory(BinaryData::drone_lowbattery_png, BinaryData::drone_lowbattery_pngSize);
		else*/ return ImageCache::getFromMemory(BinaryData::drone_ok_png, BinaryData::drone_ok_pngSize);
	}

	break;

	case Drone::WARNING: return ImageCache::getFromMemory(BinaryData::drone_warning_png, BinaryData::drone_warning_pngSize);

		/*
		case Drone::ERROR:
			return ImageCache::getFromMemory(BinaryData::drone_error_png, BinaryData::drone_error_pngSize);
			*/
	}

	return Image();
}

Image VizImages::getDroneOverlayImage(Drone* d)
{
	Drone::State s = d->state->getValueDataAsEnum<Drone::State>();
	switch (s)
	{
		/*
		case Drone::CALIBRATING:
			return ImageCache::getFromMemory(BinaryData::calibrating_png, BinaryData::calibrating_pngSize);

		case Drone::HEALTH_CHECK:
			return ImageCache::getFromMemory(BinaryData::health_analysis_png, BinaryData::health_analysis_pngSize);

		case Drone::WARNING:
			if (d->selfTestProblem->boolValue()) return ImageCache::getFromMemory(BinaryData::config_problem_png, BinaryData::config_problem_pngSize);
			if (d->batteryProblem->boolValue()) return ImageCache::getFromMemory(BinaryData::battery_problem_png, BinaryData::battery_problem_pngSize);
			if (!d->isCalibrated) return ImageCache::getFromMemory(BinaryData::badcalib_png, BinaryData::badcalib_pngSize);
			if (d->upsideDown->boolValue()) return ImageCache::getFromMemory(BinaryData::upsidedown_png, BinaryData::upsidedown_pngSize);
			break;
		*/

	case Drone::TAKING_OFF:
		return ImageCache::getFromMemory(BinaryData::startup_png, BinaryData::startup_pngSize);

	case Drone::FLYING:
		return ImageCache::getFromMemory(BinaryData::flying_png, BinaryData::flying_pngSize);

	case Drone::LANDING:
		return ImageCache::getFromMemory(BinaryData::parachute_png, BinaryData::parachute_pngSize);
	}

	return Image();
}
