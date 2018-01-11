/*
==============================================================================

This file was auto-generated!

==============================================================================
*/

#include "MainComponent.h"

#include "DroneManagerUI.h"
#include "ControllerManagerUI.h"
#include "NodeManagerUI.h"

//==============================================================================
MainContentComponent::MainContentComponent()
{

	setSize(800, 600);

	Engine::mainEngine->addEngineListener(this);

	lookAndFeelOO = new LookAndFeelOO();
	LookAndFeel::setDefaultLookAndFeel(lookAndFeelOO);

	
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Drones", &DroneManagerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Controllers", &ControllerManagerUI::create));
	ShapeShifterFactory::getInstance()->defs.add(new ShapeShifterDefinition("Nodes",&NodeManagerUI::create));
	

	ShapeShifterManager::getInstance()->setDefaultFileData(BinaryData::default_cflayout);

	ShapeShifterManager::getInstance()->setLayoutInformations("cflayout", "LaMoucheFolle/layouts");
	ShapeShifterManager::getInstance()->loadLastSessionLayoutFile();
	

	(&getCommandManager())->registerAllCommandsForTarget(this);
	(&getCommandManager())->setFirstCommandTarget(this);

	(&getCommandManager())->getKeyMappings()->resetToDefaultMappings();
	addKeyListener((&getCommandManager())->getKeyMappings());


#if JUCE_MAC
	setMacMainMenu(this, nullptr, "");
#else
	//setMenu (this); //done in Main.cpp as it's a method of DocumentWindow
#endif

}



void MainContentComponent::startLoadFile()
{

	if (fileProgressWindow != nullptr)
	{
		removeChildComponent(fileProgressWindow);
		fileProgressWindow = nullptr;
	}

	fileProgressWindow = new ProgressWindow("Loading File...", Engine::mainEngine);
	addAndMakeVisible(fileProgressWindow);
	fileProgressWindow->setSize(getWidth(), getHeight());
}

void MainContentComponent::fileProgress(float percent, int state)
{
	if (fileProgressWindow != nullptr)
	{
		fileProgressWindow->setProgress(percent);
	} else
	{
		DBG("Window is null but still got progress");
	}
}

void MainContentComponent::endLoadFile()
{
	if (fileProgressWindow != nullptr)
	{
		removeChildComponent(fileProgressWindow);
		fileProgressWindow = nullptr;
	}
}



MainContentComponent::~MainContentComponent()
{
#if JUCE_MAC
	setMacMainMenu(nullptr, nullptr, "");
#endif

	if (Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
	ShapeShifterManager::deleteInstance();

}

void MainContentComponent::init()
{
	addAndMakeVisible(&ShapeShifterManager::getInstance()->mainContainer);
}

void MainContentComponent::paint(Graphics& g)
{
	g.fillAll(BG_COLOR.darker());
}

void MainContentComponent::resized()
{
	Rectangle<int> r = getLocalBounds();
	ShapeShifterManager::getInstance()->mainContainer.setBounds(r);
}
