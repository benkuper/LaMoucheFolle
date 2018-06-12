/*
==============================================================================

This file was auto-generated!

==============================================================================
*/

#include "MainComponent.h"

#include "DroneManagerGridUI.h"
#include "ControllerManagerUI.h"
#include "NodeManagerUI.h"

//==============================================================================
MainContentComponent::MainContentComponent() :
	OrganicMainContentComponent()
{ 

	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Drones", &DroneManagerGridUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Controllers", &ControllerManagerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Nodes",&NodeManagerUI::create));
	
	ShapeShifterManager::getInstance()->setDefaultFileData(BinaryData::default_cflayout);

	ShapeShifterManager::getInstance()->setLayoutInformations("cflayout", "LaMoucheFolle/layouts");
	ShapeShifterManager::getInstance()->loadLastSessionLayoutFile();
}