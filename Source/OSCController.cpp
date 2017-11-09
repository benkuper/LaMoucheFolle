/*
  ==============================================================================

    OSCOutput.cpp
    Created: 19 Oct 2017 7:35:43pm
    Author:  Ben

  ==============================================================================
*/

#include "OSCController.h"

OSCController::OSCController(var params) :
	Controller("OSC")
{
	localPort = addIntParameter("Local Port", "Local Port to bind to receive OSC Messages", 13000, 1024, 65535);
	localPort->hideInOutliner = true;
	localPort->isTargettable = false;

	isConnected = addBoolParameter("Is Connected", "Is the receiver bound the the local port", false);
	isConnected->isEditable = false;
	isConnected->hideInOutliner = true;
	isConnected->isTargettable = false;
	isConnected->isSavable = false;

	receiver.addListener(this);
	setupReceiver();


	//Send
	useLocal = addBoolParameter("Local", "Send to Local IP (127.0.0.1). Allow to quickly switch between local and remote IP.", true);
	remoteHost = addStringParameter("Remote Host", "Remote Host to send to.", "127.0.0.1");
	remotePort = addIntParameter("Remote port", "Port on which the remote host is listening to", 13001, 1024, 65535);

	setupSender();

	//Script
}


OSCController::~OSCController()
{
}


void OSCController::sendFeedback(Drone * d, Controllable * c)
{
}

void OSCController::processMessage(const OSCMessage & msg)
{
	if (logIncomingData->boolValue())
	{
		String s = "";
		for (auto &a : msg) s += String(" ") + getStringArg(a);
		NLOG(niceName, msg.getAddressPattern().toString() << " :" << s);
	}

	inTrigger->trigger();

}


void OSCController::setupReceiver()
{
	bool result = receiver.connect(localPort->intValue());
	isConnected->setValue(result);

	if (result)
	{
		NLOG(niceName, "Now receiving on port : " + localPort->stringValue());
	}
	else
	{
		NLOG(niceName, "Error binding port " + localPort->stringValue());
	}

	Array<IPAddress> ad;
	IPAddress::findAllAddresses(ad);

	String s = "Local IPs:";
	for (auto &a : ad) s += String("\n > ") + a.toString();
	NLOG(niceName, s);
}

float OSCController::getFloatArg(OSCArgument a)
{
	if (a.isFloat32()) return a.getFloat32();
	if (a.isInt32()) return (float)a.getInt32();
	if (a.isString()) return a.getString().getFloatValue();
	return 0;
}

int OSCController::getIntArg(OSCArgument a)
{
	if (a.isInt32()) return a.getInt32();
	if (a.isFloat32()) return roundFloatToInt(a.getFloat32());
	if (a.isString()) return a.getString().getIntValue();
	return 0;
}

String OSCController::getStringArg(OSCArgument a)
{
	if (a.isString()) return a.getString();
	if (a.isInt32()) return String(a.getInt32());
	if (a.isFloat32()) return String(a.getFloat32());
	return String::empty;
}


void OSCController::setupSender()
{
	String targetHost = useLocal->boolValue() ? "127.0.0.1" : remoteHost->stringValue();
	sender.connect(targetHost, remotePort->intValue());
}

void OSCController::sendOSC(const OSCMessage & msg)
{
	if (!enabled->boolValue()) return;

	if (logOutgoingData->boolValue())
	{
		NLOG(niceName, "Send OSC : " << msg.getAddressPattern().toString());
		for (auto &a : msg)
		{
			LOG(getStringArg(a));
		}
	}

	outTrigger->trigger();
	sender.send(msg);
}



void OSCController::oscMessageReceived(const OSCMessage & message)
{
	if (!enabled->boolValue()) return;
	processMessage(message);
}

void OSCController::oscBundleReceived(const OSCBundle & bundle)
{
	if (!enabled->boolValue()) return;
	for (auto &m : bundle)
	{
		processMessage(m.getMessage());
	}
}
