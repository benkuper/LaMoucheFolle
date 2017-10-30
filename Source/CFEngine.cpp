/*
==============================================================================

Engine.cpp
Created: 2 Apr 2016 11:03:21am
Author:  Martin Hermant

==============================================================================
*/
#include "CFEngine.h"
#include "DroneManager.h"
#include "ControllerManager.h"

CFEngine::CFEngine(ApplicationProperties * appProperties, const String &appVersion) :
	Engine("CrazyflieServer", ".crazyflie", appProperties, appVersion)
{
	//init here
	Engine::mainEngine = this;
	
	addChildControllableContainer(DroneManager::getInstance());
	addChildControllableContainer(ControllerManager::getInstance());
}


CFEngine::~CFEngine()
{
	//Application-end cleanup, nothing should be recreated after this

	//delete singletons here
	
	ControllerManager::deleteInstance();
	DroneManager::deleteInstance();

}

void CFEngine::clearInternal()
{
	//clear

	ControllerManager::getInstance()->clear();
	DroneManager::getInstance()->clear();


}

var CFEngine::getJSONData()
{
	var data = Engine::getJSONData();

	//save here
	data.getDynamicObject()->setProperty("droneManager", DroneManager::getInstance()->getJSONData());
	data.getDynamicObject()->setProperty("controllerManager", ControllerManager::getInstance()->getJSONData());

	return data;
}

void CFEngine::loadJSONDataInternalEngine(var data, ProgressTask * loadingTask)
{
	ProgressTask * droneTask = loadingTask->addTask("Drones");
	ProgressTask * controllerTask = loadingTask->addTask("Controllers");

	//load here
	droneTask->start();
	DroneManager::getInstance()->loadJSONData(data.getProperty("droneManager", var()));
	droneTask->setProgress(1);
	droneTask->end();

	controllerTask->start();
	ControllerManager::getInstance()->loadJSONData(data.getProperty("controllerManager", var()));
	controllerTask->setProgress(1);
	controllerTask->end();
}


String CFEngine::getMinimumRequiredFileVersion()
{
	return "1.0.0";
}
