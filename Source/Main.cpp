#include "Main.h"

String getAppVersion();

CFApplication::~CFApplication()
{
	CFSettings::deleteInstance();
}

void CFApplication::initialiseInternal(const String &)
{
	engine.reset(new CFEngine());
	mainComponent.reset(new MainContentComponent());

	ProjectSettings::getInstance()->addChildControllableContainer(CFSettings::getInstance());
}
