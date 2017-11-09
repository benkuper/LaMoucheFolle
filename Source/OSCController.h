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

	void sendFeedback(Drone * d, Controllable * c) override;
	void processMessage(const OSCMessage & msg);

	//RECEIVE
	void setupReceiver();
	float getFloatArg(OSCArgument a);
	int getIntArg(OSCArgument a);
	String getStringArg(OSCArgument a);

	//SEND
	void setupSender();
	void sendOSC(const OSCMessage &msg);

	
	virtual void oscMessageReceived(const OSCMessage & message) override;
	virtual void oscBundleReceived(const OSCBundle & bundle) override;


	String getTypeString() const override { return "OSC"; }
	static OSCController * create(var params) { return new OSCController(params); }

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCController)

};