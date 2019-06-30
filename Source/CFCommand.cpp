/*
  ==============================================================================

	CFCommand.cpp
	Created: 19 Jun 2018 9:04:27am
	Author:  Ben

  ==============================================================================
*/

/*
#include "CFCommand.h"
#include "CFDrone.h"
#include "CFParam.h"

CFCommand::CFCommand(CFDrone * drone, Array<uint8> _data, Type type) :
	drone(drone),
	data(_data),
	type(type)
{

}

CFCommand * CFCommand::createPing(CFDrone * d) {
	return new CFCommand(d, Array<uint8>((uint8)0xFF), PING);
}
CFCommand * CFCommand::createStop(CFDrone * d)
{
	crtpStopRequest r;
	return  new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpStopRequest)), STOP);
}

CFCommand * CFCommand::createRebootInit(CFDrone * d)
{
	return new CFCommand(d, Array<uint8>((uint8)0xFF, (uint8)0xFE, (uint8)0xFF), REBOOT_INIT);
}

CFCommand * CFCommand::createRebootFirmware(CFDrone * d)
{
	return new CFCommand(d, Array<uint8>((uint8)0xFF, (uint8)0xFE, (uint8)0xF0, (uint8)0x01), REBOOT_FIRMWARE);
}

CFCommand * CFCommand::createSetPoint(CFDrone * d, float yaw, float pitch, float roll, float thrust) {
	crtpSetpointRequest r(roll, pitch, yaw, (uint16_t)thrust);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpSetpointRequest)), SETPOINT);
}
CFCommand * CFCommand::createVelocity(CFDrone * d, Vector3D<float> vel, float yaw) {
	crtpVelocitySetpointRequest r(vel.x, vel.y, vel.z, yaw);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpVelocitySetpointRequest)), VELOCITY);
}
CFCommand * CFCommand::createPosition(CFDrone * d, Vector3D<float> pos, float yaw) {
	crtpPositionSetpointRequest r(pos.x, pos.y, pos.z, yaw);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpPositionSetpointRequest)), POSITION);
}

CFCommand * CFCommand::createSetParam(CFDrone * d, StringRef name, var value) {
	CFParamToc * toc = d->paramToc;
	if (toc == nullptr)
	{
		DBG("Drone has not param toc set !");
		return nullptr;
	}

	CFParam * p = toc->getParam(name);

	if (p == nullptr)
	{
		DBG("Param not found " << name);
		return nullptr;
	}

	Array<uint8> data;
	switch (p->definition.type)
	{
	case CFParam::Type::Uint8: { crtpParamWriteRequest<uint8_t> r(p->definition.id, (uint8_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteRequest<uint8_t>)); } break;

	case CFParam::Type::Int8: { crtpParamWriteRequest<int8_t> r(p->definition.id, (int8_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteRequest<int8_t>)); }	break;
	case CFParam::Type::Uint16: { crtpParamWriteRequest<uint16_t> r(p->definition.id, (uint16_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteRequest<uint16_t>));  }break;
	case CFParam::Type::Int16: { crtpParamWriteRequest<int16_t> r(p->definition.id, (int16_t)(int)value);  data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteRequest<int16_t>));  }break;
	case CFParam::Type::Uint32: { crtpParamWriteRequest<uint32_t> r(p->definition.id, (uint32_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteRequest<uint32_t>)); }break;
	case CFParam::Type::Int32: { crtpParamWriteRequest<int32_t> r(p->definition.id, value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteRequest<int32_t>));  }break;
	case CFParam::Type::Float: { crtpParamWriteRequest<float> r(p->definition.id, value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteRequest<float>));  }break;
	default: DBG("Type not handled : " << (int)p->definition.type); return nullptr; break;
	}


	return new CFCommand(d, data, SET_PARAM);
}


CFCommand * CFCommand::createGetParamValue(CFDrone * d, StringRef name)
{
	CFParamToc * toc = d->paramToc;
	if (toc == nullptr)
	{
		DBG("Drone has not param toc set !");
		return nullptr;
	}

	CFParam * p = toc->getParam(name);

	if (p == nullptr)
	{
		DBG("Param not found " << name);
		return nullptr;
	}
	crtpParamReadRequest r(p->definition.id);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamReadRequest)), GET_PARAM_VALUE);
}

CFCommand * CFCommand::createGetParamValue(CFDrone * d, int paramId)
{
	crtpParamReadRequest r(paramId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamReadRequest)), GET_PARAM_VALUE);
}

CFCommand * CFCommand::createGetParamInfo(CFDrone * d, int paramId)
{
	DBG("Create param getInfo command for id " << paramId);
	crtpParamTocGetItemRequest r(paramId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamTocGetItemRequest)), GET_PARAM_INFO);
}


CFCommand * CFCommand::createRequestParamToc(CFDrone * d)
{
	crtpParamTocGetInfoRequest r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamTocGetInfoRequest)), REQUEST_PARAM_TOC);
}

CFCommand * CFCommand::createActivateSafeLink(CFDrone * d)
{
	return new CFCommand(d, Array<uint8>(Crazyradio::safeLinkPacket, sizeof(Crazyradio::safeLinkPacket)), ACTIVATE_SAFELINK);
}

CFCommand * CFCommand::createRequestLogToc(CFDrone * d)
{
	crtpLogGetInfoRequest r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogGetInfoRequest)), REQUEST_LOG_TOC);
}

CFCommand * CFCommand::createGetLogItemInfo(CFDrone * d, int id)
{
	crtpLogGetItemRequest r(id);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogGetItemRequest)), GET_LOG_ITEM_INFO);
}

CFCommand * CFCommand::createLPSNodePos(CFDrone * d, int nodeId, float x, float y, float z)
{
	crtpLppSetNodePosRequest r(nodeId, x, y, z);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLppSetNodePosRequest)), LPS_NODE_POS_SET);
}

CFCommand * CFCommand::createGetMemoryNumber(CFDrone * d)
{
	crtpMemoryGetNumberRequest r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryGetNumberRequest)), GET_MEMORY_NUMBER);
}

CFCommand * CFCommand::createGetMemoryInfo(CFDrone * d, int memoryId)
{
	crtpMemoryGetInfoRequest r((uint8)memoryId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryGetInfoRequest)), GET_MEMORY_INFO);
}

CFCommand * CFCommand::createReadMemory(CFDrone * d, int memoryId, int memAddress, int length)
{
	crtpMemoryReadRequest r((uint8)memoryId, memAddress, (uint8)length);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryReadRequest)), READ_MEMORY);
}

CFCommand * CFCommand::createWriteMemory(CFDrone * d, int memoryId, int memAddress, Array<uint8> data)
{
	crtpMemoryWriteRequest r((uint8)memoryId, memAddress);
	for (int i = 0; i < data.size() && i < 24; i++) r.data[i] = data[i];
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryWriteRequest)), WRITE_MEMORY);
}

CFCommand * CFCommand::createResetLogs(CFDrone * d)
{
	crtpLogResetRequest r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogResetRequest)), RESET_LOGS);
}

CFCommand * CFCommand::createAddLogBlock(CFDrone * d, int logBlockId, Array<String> variableNames)
{
	crtpLogCreateBlockRequest r;
	r.id = logBlockId;

	for (int i = 0; i < variableNames.size(); i++)
	{
		CFLogVariable * v = d->logToc->getLogVariable(variableNames[i]);
		if (v == nullptr)
		{
			LOGWARNING("Variable " << variableNames[i] << " not found in toc");
			continue;
		}
		r.items[i].id = v->definition.id;
		r.items[i].logType = v->definition.type;
	}

	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogCreateBlockRequest)), ADD_LOG_BLOCK);
}

CFCommand * CFCommand::createStartLog(CFDrone * d, int logBlockId, int freq)
{
	crtpLogStartRequest r(logBlockId, 100 / freq);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogStartRequest)), START_LOG);
}

CFCommand * CFCommand::createStopLog(CFDrone * d, int logBlockId)
{
	crtpLogStopRequest r(logBlockId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogStopRequest)), STOP_LOG);
}

*/