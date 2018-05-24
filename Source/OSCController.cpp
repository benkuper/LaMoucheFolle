/*
  ==============================================================================

    OSCOutput.cpp
    Created: 19 Oct 2017 7:35:43pm
    Author:  Ben

  ==============================================================================
*/

#include "OSCController.h"

#include "NodeManager.h"
#include "DroneManager.h"
#include <type_traits>

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

void OSCController::sendDroneFeedback(Drone2 * d, Controllable * c)
{

	Controller::sendDroneFeedback(d, c);
	Parameter * p = dynamic_cast<Parameter *>(c);
	if (p != nullptr)
	{
		OSCMessage m("/drone-" + d->shortName + "/"+p->shortName);
		if (p->value.isArray())
		{
			for (int i = 0; i < p->value.size(); i++)
			{
				m.addArgument(varToArgument(p->value[i]));
			}
		}
		else
		{
			if (p->type == Controllable::ENUM) m.addArgument(varToArgument(((EnumParameter*)p)->getValueData()));
			else m.addArgument(varToArgument(p->value));
		}
		sendOSC(m);
	}
}

void OSCController::sendNodeFeedback(Node * n, Controllable * c)
{
	DBG("Drone::Send Node feedback " << c->shortName);
	Controller::sendNodeFeedback(n, c);
	Parameter * p = dynamic_cast<Parameter *>(c);
	if (p != nullptr)
	{
		OSCMessage m("/node-" + n->shortName + "/" + p->shortName);
		if (p->value.isArray())
		{
			for (int i = 0; i < p->value.size(); i++)
			{
				m.addArgument(varToArgument(p->value[i]));
			}
		}
		else
		{
			if (p->type == Controllable::ENUM) m.addArgument(varToArgument(((EnumParameter*)p)->getValueData()));
			else m.addArgument(varToArgument(p->value));
		}sendOSC(m);
	}
	
}

void OSCController::sendFullSetup()
{
	
	OSCMessage m("/drones/setup");
	for (Drone2 * d : DroneManager::getInstance()->items) m.addString(d->shortName);
	sendOSC(m);
	
	OSCMessage m2("/nodes/setup");
	for (Node * n : NodeManager::getInstance()->items) m2.addString(n->shortName);
	sendOSC(m2);
	
}

void OSCController::sendDroneSetup(const String & droneName)
{
	
	Drone2 * d = DroneManager::getInstance()->getItemWithName(droneName);
	if (d == nullptr)
	{
		DBG("Drone " + droneName + " doesn't exist");
		return;
	}

	sendDroneFeedback(d, d->state); 
	sendDroneFeedback(d, d->realPosition);
	//sendDroneFeedback(d, d->lowBattery);
	//sendDroneFeedback(d, d->charging);
	sendDroneFeedback(d, d->lightMode);
	sendDroneFeedback(d, d->color); //no support for automatic color for now
	
}

void OSCController::sendNodeSetup(const String &nodeName)
{
	Node * n = NodeManager::getInstance()->getItemWithName(nodeName);
	if (n == nullptr)
	{
		DBG("Node " + nodeName + " doesn't exist : " <<nodeName);
		return;
	}

	sendNodeFeedback(n, n->id);
	sendNodeFeedback(n, n->position);
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

	StringArray tokens;
	tokens.addTokens(msg.getAddressPattern().toString(), "/", "\"");
	
	

	if (tokens[1] == "setup") sendFullSetup();
	else if (tokens[1].contains("drone"))
	{
		String droneName = tokens[1].substring(6);
		if(tokens[2] == "setup") sendDroneSetup(droneName);
		else
		{
			
			Drone2 * d = DroneManager::getInstance()->getItemWithName(droneName); 
			Controllable * c = d->getControllableByName(tokens[2]);
			//DBG("Find controllable : " << tokens[2] << " / " << (int)( c != nullptr));
			
			if (c != nullptr)
			{
				handleSetControllableValue(c, msg);
			}
		}
	}
	else if (tokens[1].contains("node"))
	{
		DBG("Tokens : " << tokens.joinIntoString(", "));
		String nodeName = tokens[1].substring(5);
		if (tokens[2] == "setup") sendNodeSetup(nodeName);
	}

}

void OSCController::handleSetControllableValue(Controllable * c, const OSCMessage & msg)
{
	switch (c->type)
	{
	case Controllable::TRIGGER:
		((Trigger *)c)->trigger();
		break;



	case Controllable::BOOL:
		((Parameter *)c)->setValue(getFloatArg(msg[0]) >= 1); break;
		break;

	case Controllable::FLOAT:
		if (msg.size() >= 1)
		{
			FloatParameter *f = (FloatParameter *)c;
			f->setValue(getFloatArg(msg[0]));
		}
		break;

	case Controllable::INT:
		if (msg.size() >= 1)
		{
			IntParameter *i = (IntParameter *)c;
			i->setValue(getIntArg(msg[0]));
		}
		break;

	case Controllable::STRING:
		if (msg.size() >= 1) ((StringParameter *)c)->setValue(getStringArg(msg[0]));
		break;

	case Controllable::POINT2D:
		if (msg.size() >= 2) ((Point2DParameter *)c)->setPoint(getFloatArg(msg[0]), getFloatArg(msg[1]));
		break;

	case Controllable::POINT3D:
		if (msg.size() >= 3) ((Point3DParameter *)c)->setVector(Vector3D<float>(getFloatArg(msg[0]), getFloatArg(msg[1]), getFloatArg(msg[2])));
		break;

	case Controllable::COLOR:
		if (msg.size() >= 4) ((ColorParameter *)c)->setColor(getColorArg(msg[0], msg[1], msg[2], msg[3]));
		if (msg.size() >= 3) ((ColorParameter *)c)->setColor(getColorArg(msg[0],msg[1],msg[2]));
		break;
	
	case Controllable::ENUM:
		if (msg.size() >= 1) ((EnumParameter *)c)->setValueWithData(getIntArg(msg[0]));
		break;

	default:
		DBG("Not handled " << c->type);
		break;
	}
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
	if (a.isFloat32()) return roundToInt(a.getFloat32());
	if (a.isString()) return a.getString().getIntValue();
	return 0;
}

Colour OSCController::getColorArg(OSCArgument r, OSCArgument g, OSCArgument b, OSCArgument a)
{
	return Colour((uint8)(r.getFloat32() * 255), (uint8)(g.getFloat32() * 255), (uint8)(b.getFloat32() * 255), (uint8)(a.getFloat32() * 255));
}

Colour OSCController::getColorArg(OSCArgument r, OSCArgument g, OSCArgument b)
{
	return Colour((uint8)(r.getFloat32() * 255), (uint8)(g.getFloat32() * 255), (uint8)(b.getFloat32() * 255), (uint8)255);
}

String OSCController::getStringArg(OSCArgument a)
{
	if (a.isString()) return a.getString();
	if (a.isInt32()) return String(a.getInt32());
	if (a.isFloat32()) return String(a.getFloat32());
	return String();
}

OSCArgument OSCController::varToArgument(const var & v)
{
	if (v.isBool()) return OSCArgument(((bool)v) ? 1 : 0);
	else if (v.isInt()) return OSCArgument((int)v);
	else if (v.isInt64()) return OSCArgument((int)v);
	else if (v.isDouble()) return OSCArgument((float)v);
	else if (v.isString()) return OSCArgument(v.toString());
	jassert(false);
	return OSCArgument("error");
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
