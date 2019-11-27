/*
==============================================================================

This file was auto-generated!

==============================================================================
*/

#include "MainComponent.h"

#include "Drone/UI/DroneManagerUI.h"
#include "Drone/UI/DroneManagerGridUI.h"
#include "Drone/UI/Drone2DViewManagerUI.h"
#include "controller/ui/ControllerManagerUI.h"
//#include "NodeManagerUI.h"

//==============================================================================
MainContentComponent::MainContentComponent() :
	OrganicMainContentComponent()
{ 

	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Drone List", &DroneManagerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Drone Grid", &DroneManagerGridUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Drone 2D View", &DroneManager2DViewUI::create));

	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Controllers", &ControllerManagerUI::create));
	//ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Nodes",&NodeManagerUI::create));
	
	ShapeShifterManager::getInstance()->setDefaultFileData(BinaryData::default_cflayout);

	ShapeShifterManager::getInstance()->setLayoutInformations("cflayout", "LaMoucheFolle/layouts");
	ShapeShifterManager::getInstance()->loadLastSessionLayoutFile();
}