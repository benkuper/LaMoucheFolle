/*
  ==============================================================================

    OSCOutput.h
    Created: 19 Oct 2017 7:35:43pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "Controller.h"

class OSCController :
	public Controller,
	public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
	OSCController(var params = var());
	~OSCController();

	//RECEIVE
	IntParameter * localPort;
	BoolParameter * isConnected;
	OSCReceiver receiver;

	//SEND
	BoolParameter * useLocal;
	StringParameter * remoteHost;
	IntParameter * remotePort;
	OSCSender sender;

	void sendDroneFeedback(Drone2 * d, Controllable * c) override;
	void sendNodeFeedback(Node * d, Controllable * c) override;

	void sendFullSetup() override;
	void sendDroneSetup(const String &droneName) override;
	void sendNodeSetup(const String &nodeName) override;

	void processMessage(const OSCMessage & msg);

	void handleSetControllableValue(Controllable *c, const OSCMessage &msg);

	//RECEIVE
	void setupReceiver();
	float getFloatArg(OSCArgument a);
	int getIntArg(OSCArgument a);
	Colour getColorArg(OSCArgument r, OSCArgument g, OSCArgument b, OSCArgument a);
	Colour getColorArg(OSCArgument r,OSCArgument g, OSCArgument b);
	String getStringArg(OSCArgument a);
	OSCArgument varToArgument(const var &v);

	//SEND
	void setupSender();
	void sendOSC(const OSCMessage &msg);

	
	virtual void oscMessageReceived(const OSCMessage & message) override;
	virtual void oscBundleReceived(const OSCBundle & bundle) override;


	String getTypeString() const override { return "OSC"; }
	static OSCController * create(var params) { return new OSCController(params); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCController)

};