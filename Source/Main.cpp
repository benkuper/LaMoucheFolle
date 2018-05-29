#include "Main.h"

String getAppVersion();

void CFApplication::initialiseInternal(const String &)
{
	engine = new CFEngine(appProperties, getAppVersion());
	mainComponent = new MainContentComponent();
}
