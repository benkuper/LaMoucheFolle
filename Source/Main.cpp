#include "Main.h"

String getAppVersion();

void CFApplication::initialiseInternal(const String &)
{
	engine = new CFEngine();
	mainComponent = new MainContentComponent();
}
