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

class Drone;

class CFCommand
{
public:
	enum Type { PING, SETPOINT, POSITION, TAKEOFF, LAND, SET_PARAM, GET_PARAM_INFO, GET_PARAM_VALUE, REQUEST_PARAM_TOC, RESET_LOGS, ADD_LOG_BLOCK, START_LOG, STOP_LOG, REQUEST_LOG_TOC, GET_LOG_ITEM_INFO, ACTIVATE_SAFELINK, STOP, REBOOT_INIT, REBOOT_FIRMWARE, SYSTEM_OFF, SYSTEM_ON, SYSTEM_LED, LPS_NODE_POS_SET, LIGHTHOUSE_BS_GEOMETRY_SET, GET_MEMORY_NUMBER, GET_MEMORY_INFO, READ_MEMORY, WRITE_MEMORY, TYPES_MAX};
	const String typeStrings[TYPES_MAX] { "Ping","SetPoint","Position","TakeOff", "Land", "SetParam","GetParamInfo","GetParamValue","RequestParamToc","ResetLogs","AddLogBlock", "StartLog", "StopLog", "RequestLogToc", "GetLogItemInfo", "SafeLink", "Stop","RebootInit","RebootFirmware", "SystemOff","SystemOn","SystemLed", "LPSNodePosSet", "LightHouseBSGeometrySet", "GetMemoryNumber","GetMemoryInfo","ReadMemory","WriteMemory" };

	CFCommand(Drone * drone, Array<uint8> data, Type type);
	WeakReference<Drone> drone;
	Array<uint8_t> data;
	Type type;

	static CFCommand * createPing(Drone * d);
	static CFCommand * createStop(Drone * d);
	static CFCommand * createRebootInit(Drone * d);
	static CFCommand*  createRebootFirmware(Drone* d);
	static CFCommand*  createSystemOff(Drone* d);
	static CFCommand*  createSystemOn(Drone* d);
	static CFCommand * createSystemLed(Drone * d, bool isOn);
	static CFCommand * createSetPoint(Drone * d, float yaw, float pitch, float roll, float thrust);
	static CFCommand * createPosition(Drone* d, Vector3D<float> pos, float yaw);
	static CFCommand * createHighLevelTakeOff(Drone* d, float targetHeight, float time);
	static CFCommand*  createHighLevelLand(Drone* d, float targetHeight, float time);
	static CFCommand* createHighLevelGoto(Drone* d, Vector3D<float> pos, float yaw, float time);
	static CFCommand * createHighLevelStop(Drone* d);
	/*static CFCommand * createHighLevelDefineTrajectory(Drone* d, float height, float time);
	static CFCommand * createHighLevelStartTrajectory(Drone * d, float height, float time);*/
	static CFCommand * createSetParam(Drone * d, StringRef name, var value);
	static CFCommand * createGetParamInfo(Drone * d, int paramId);
	static CFCommand * createGetParamValue(Drone * d, StringRef name);
	static CFCommand * createGetParamValue(Drone * d, int id);
	static CFCommand * createRequestParamToc(Drone * d);
	static CFCommand * createActivateSafeLink(Drone * d);
	static CFCommand * createRequestLogToc(Drone * d);
	static CFCommand * createGetLogItemInfo(Drone * d, int id);
	//static CFCommand*  createLPSNodePos(Drone* d, int nodeId, Vector3D<float> position);
	//static CFCommand * createBSGeometry(Drone * d, int nodeId, Vector3D<float> origin, Vector3D<float> mat1, Vector3D<float> mat2);
	static CFCommand * createGetMemoryNumber(Drone * d);
	static CFCommand * createGetMemoryInfo(Drone * d, int memoryId);
	static CFCommand * createReadMemory(Drone * d, int memoryId, int memAddress, int length);
	static CFCommand * createWriteMemory(Drone * d, int memoryId, int memAddress, Array<uint8> data);

	static CFCommand * createResetLogs(Drone * d);
	static CFCommand * createAddLogBlock(Drone * d, int logBlockId, Array<String> variableNames);
	static CFCommand * createStartLog(Drone * d, int logBlockId, int freq);
	static CFCommand * createStopLog(Drone * d, int logBlockId);

	String getTypeString() const { return typeStrings[type]; } 

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CFCommand)
};