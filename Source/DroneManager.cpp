/*
  ==============================================================================

    DroneManager.cpp
    Created: 19 Oct 2017 7:33:19pm
    Author:  Ben

  ==============================================================================
*/

#include "DroneManager.h"

juce_ImplementSingleton(DroneManager)

DroneManager::DroneManager() :
	BaseManager("Drones")
{

}

DroneManager::~DroneManager()
{
}
