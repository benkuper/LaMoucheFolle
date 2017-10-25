/*
==============================================================================

Engine.cpp
Created: 2 Apr 2016 11:03:21am
Author:  Martin Hermant

==============================================================================
*/
#include "CFEngine.h"
#include "DroneManager.h"

CFEngine::CFEngine(ApplicationProperties * appProperties, const String &appVersion) :
	Engine("CrazyflieServer", ".crazyflie", appProperties, appVersion)
{
	//init here
	Engine::mainEngine = this;
	
	addChildControllableContainer(DroneManager::getInstance());
}


CFEngine::~CFEngine()
{
	//Application-end cleanup, nothing should be recreated after this

	//delete singletons here
	
	DroneManager::deleteInstance();

	/*
	ModuleRouterManager::deleteInstance();

	SequenceManager::deleteInstance();
	StateManager::deleteInstance();
	ModuleManager::deleteInstance();

	MappingFilterFactory::deleteInstance();
	ConditionFactory::deleteInstance();
	ProcessorFactory::deleteInstance();

	MIDIManager::deleteInstance();
	DMXManager::deleteInstance();
	SerialManager::deleteInstance();
	GamepadManager::deleteInstance();
	WiimoteManager::deleteInstance();
	*/

}

void CFEngine::clearInternal()
{
	//clear

	DroneManager::getInstance()->clear();

	/*
	StateManager::getInstance()->clear();
	SequenceManager::getInstance()->clear();

	ModuleRouterManager::getInstance()->clear();
	ModuleManager::getInstance()->clear();
	*/

}

var CFEngine::getJSONData()
{
	var data = Engine::getJSONData();

	//save here
	
	data.getDynamicObject()->setProperty("droneManager", DroneManager::getInstance()->getJSONData());
	
	/*
	data.getDynamicObject()->setProperty("stateManager", StateManager::getInstance()->getJSONData());
	data.getDynamicObject()->setProperty("sequenceManager", SequenceManager::getInstance()->getJSONData());
	data.getDynamicObject()->setProperty("routerManager", ModuleRouterManager::getInstance()->getJSONData());
	*/

	return data;
}

void CFEngine::loadJSONDataInternalEngine(var data, ProgressTask * loadingTask)
{
	ProgressTask * droneTask = loadingTask->addTask("Drones");


	//load here
	
	droneTask->start();
	DroneManager::getInstance()->loadJSONData(data.getProperty("droneManager", var()));
	droneTask->setProgress(1);
	droneTask->end();

	/*
	stateTask->start();
	StateManager::getInstance()->loadJSONData(data.getProperty("stateManager", var()));
	stateTask->setProgress(1);
	stateTask->end();

	sequenceTask->start();
	SequenceManager::getInstance()->loadJSONData(data.getProperty("sequenceManager", var()));
	sequenceTask->setProgress(1);
	sequenceTask->end();

	routerTask->start();
	ModuleRouterManager::getInstance()->loadJSONData(data.getProperty("routerManager", var()));
	routerTask->setProgress(1);
	routerTask->end();
	*/

}

String CFEngine::getMinimumRequiredFileVersion()
{
	return "1.0.0";
}
