/*
  ==============================================================================

    CFCommand.h
    Created: 19 Jun 2018 9:04:27am
    Author:  Ben

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "crtp.h"
class CFDrone;

class CFCommand
{
public:
	enum Type { PING, SETPOINT, POSITION, VELOCITY, SET_PARAM, REQUEST_PARAM, REQUEST_PARAM_TOC, SET_LOG, START_LOC, REQUEST_LOG_TOC, ACTIVATE_SAFELINK, TYPES_MAX};
	const String typeStrings[TYPES_MAX] { "Ping","SetPoint","Position","Velocity","SetParam","RequestParam","RequestParamToc","SetLog","StartLoc","RequestLogToc", "SafeLink" };

	CFCommand(CFDrone * drone, Array<uint8> data, Type type);
	WeakReference<CFDrone> drone;
	Array<uint8_t> data;
	Type type;

	static CFCommand * createPing(CFDrone * d);
	static CFCommand * createSetPoint(CFDrone * d, float yaw, float pitch, float roll, float thrust);
	static CFCommand * createVelocity(CFDrone * d, Vector3D<float> vel, float yaw);
	static CFCommand * createPosition(CFDrone * d, Vector3D<float> pos, float yaw);
	static CFCommand * createSetParam(CFDrone * d, StringRef name, var yaw);
	static CFCommand * createGetParam(CFDrone * d, StringRef name);
	static CFCommand * createRequestParamToc(CFDrone * d);
	static CFCommand * createActivateSafeLink(CFDrone * d);
	/*
	static CFCommand * createSetLog(CFDrone * d);
	static CFCommand * createStartLog(CFDrone * d);
	static CFCommand * createSetLog(CFDrone * d);
	static CFCommand * createRequestLogToc(CFDrone * d);
	*/


	String getTypeString() const { return typeStrings[type]; }
};