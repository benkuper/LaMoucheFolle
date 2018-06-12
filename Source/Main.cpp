#include "Main.h"

String getAppVersion();

CFApplication::~CFApplication()
{
	CFSettings::deleteInstance();
}

void CFApplication::initialiseInternal(const String &)
{
	engine = new CFEngine();
	mainComponent = new MainContentComponent();

	ProjectSettings::getInstance()->addChildControllableContainer(CFSettings::getInstance());
}
