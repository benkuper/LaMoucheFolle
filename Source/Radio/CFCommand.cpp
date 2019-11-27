/*
  ==============================================================================

	CFCommand.cpp
	Created: 19 Jun 2018 9:04:27am
	Author:  Ben

  ==============================================================================
*/


#include "CFCommand.h"
#include "Drone/Drone.h"
#include "CFParam.h"

CFCommand::CFCommand(Drone * drone, Array<uint8> _data, Type type) :
	drone(drone),
	data(_data),
	type(type)
{
	
}

CFCommand * CFCommand::createPing(Drone * d) {
	return new CFCommand(d, Array<uint8>((uint8)0xFF), PING);
}
CFCommand * CFCommand::createStop(Drone * d)
{
	crtpStopRequest r;
	return  new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpStopRequest)), STOP);
}

CFCommand * CFCommand::createRebootInit(Drone * d)
{
	return new CFCommand(d, Array<uint8>((uint8)0xFF, (uint8)0xFE, (uint8)0xFF), REBOOT_INIT);
}

CFCommand * CFCommand::createRebootFirmware(Drone * d)
{
	return new CFCommand(d, Array<uint8>((uint8)0xFF, (uint8)0xFE, (uint8)0xF0, (uint8)0x01), REBOOT_FIRMWARE);
}

CFCommand * CFCommand::createSetPoint(Drone * d, float yaw, float pitch, float roll, float thrust) {
	crtpSetpointRequest r(roll, pitch, yaw, (uint16_t)thrust);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpSetpointRequest)), SETPOINT);
}

CFCommand * CFCommand::createPosition(Drone * d, Vector3D<float> pos, float yaw) {
	crtpPositionSetpointRequest r(pos.x, pos.y, pos.z, yaw);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpPositionSetpointRequest)), POSITION);
}

CFCommand* CFCommand::createHighLevelTakeOff(Drone* d, float targetHeight, float time)
{
	crtpCommanderHighLevelTakeoffRequest r(0, targetHeight, time);
	return new CFCommand(d, Array<uint8>((uint8*)&r, sizeof(crtpCommanderHighLevelTakeoffRequest)), TAKEOFF);
}

CFCommand* CFCommand::createHighLevelLand(Drone* d, float targetHeight, float time)
{
	crtpCommanderHighLevelLandRequest r(0, targetHeight, time);
	return new CFCommand(d, Array<uint8>((uint8*)&r, sizeof(crtpCommanderHighLevelLandRequest)), LAND);
}

CFCommand* CFCommand::createHighLevelGoto(Drone* d, Vector3D<float> pos, float yaw, float time)
{
	crtpCommanderHighLevelGoToRequest r(0, false, pos.x, pos.y, pos.z, yaw, time);
	return new CFCommand(d, Array<uint8>((uint8*)&r, sizeof(crtpCommanderHighLevelGoToRequest)), LAND);

}

CFCommand* CFCommand::createHighLevelStop(Drone* d)
{
	crtpCommanderHighLevelStopRequest r(0);
	return new CFCommand(d, Array<uint8>((uint8*)&r, sizeof(crtpCommanderHighLevelStopRequest)), LAND);
}

CFCommand * CFCommand::createSetParam(Drone * d, StringRef name, var value) {
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
	case CFParam::Type::Uint8: { crtpParamWriteV2Request<uint8_t> r(p->definition.id, (uint8_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteV2Request<uint8_t>)); } break;

	case CFParam::Type::Int8: { crtpParamWriteV2Request<int8_t> r(p->definition.id, (int8_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteV2Request<int8_t>)); }	break;
	case CFParam::Type::Uint16: { crtpParamWriteV2Request<uint16_t> r(p->definition.id, (uint16_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteV2Request<uint16_t>));  }break;
	case CFParam::Type::Int16: { crtpParamWriteV2Request<int16_t> r(p->definition.id, (int16_t)(int)value);  data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteV2Request<int16_t>));  }break;
	case CFParam::Type::Uint32: { crtpParamWriteV2Request<uint32_t> r(p->definition.id, (uint32_t)(int)value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteV2Request<uint32_t>)); }break;
	case CFParam::Type::Int32: { crtpParamWriteV2Request<int32_t> r(p->definition.id, value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteV2Request<int32_t>));  }break;
	case CFParam::Type::Float: { crtpParamWriteV2Request<float> r(p->definition.id, value); data = Array<uint8>((uint8 *)&r, sizeof(crtpParamWriteV2Request<float>));  }break;
	default: DBG("Type not handled : " << (int)p->definition.type); return nullptr; break;
	}


	return new CFCommand(d, data, SET_PARAM);
}


CFCommand * CFCommand::createGetParamValue(Drone * d, StringRef name)
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
	crtpParamReadV2Request r(p->definition.id);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamReadV2Request)), GET_PARAM_VALUE);
}

CFCommand * CFCommand::createGetParamValue(Drone * d, int paramId)
{
	crtpParamReadV2Request r(paramId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamReadV2Request)), GET_PARAM_VALUE);
}

CFCommand * CFCommand::createGetParamInfo(Drone * d, int paramId)
{
	DBG("Create param getInfo command for id " << paramId);
	crtpParamTocGetItemV2Request r(paramId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamTocGetItemV2Request)), GET_PARAM_INFO);
}


CFCommand * CFCommand::createRequestParamToc(Drone * d)
{
	crtpParamTocGetInfoV2Request r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpParamTocGetInfoV2Request)), REQUEST_PARAM_TOC);
}

CFCommand * CFCommand::createActivateSafeLink(Drone * d)
{
	return new CFCommand(d, Array<uint8>((const uint8_t *)CFPacket::safeLinkPacket,3), ACTIVATE_SAFELINK);
}

CFCommand * CFCommand::createRequestLogToc(Drone * d)
{
	crtpLogGetInfoV2Request r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogGetInfoV2Request)), REQUEST_LOG_TOC);
}

CFCommand * CFCommand::createGetLogItemInfo(Drone * d, int id)
{
	crtpLogGetItemV2Request r(id);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogGetItemV2Request)), GET_LOG_ITEM_INFO);
}
/*
CFCommand * CFCommand::createLPSNodePos(Drone * d, int nodeId, Vector3D<float> position)
{
	crtpLppSetNodePosRequest r(nodeId, position.x, position.y, position.z);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLppSetNodePosRequest)), LPS_NODE_POS_SET);
}
*/
CFCommand * CFCommand::createGetMemoryNumber(Drone * d)
{
	crtpMemoryGetNumberRequest r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryGetNumberRequest)), GET_MEMORY_NUMBER);
}

CFCommand * CFCommand::createGetMemoryInfo(Drone * d, int memoryId)
{
	crtpMemoryGetInfoRequest r((uint8)memoryId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryGetInfoRequest)), GET_MEMORY_INFO);
}

CFCommand * CFCommand::createReadMemory(Drone * d, int memoryId, int memAddress, int length)
{
	crtpMemoryReadRequest r((uint8)memoryId, memAddress, (uint8)length);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryReadRequest)), READ_MEMORY);
}

CFCommand * CFCommand::createWriteMemory(Drone * d, int memoryId, int memAddress, Array<uint8> data)
{
	crtpMemoryWriteRequest r((uint8)memoryId, memAddress);
	for (int i = 0; i < data.size() && i < 24; i++) r.data[i] = data[i];
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpMemoryWriteRequest)), WRITE_MEMORY);
}

CFCommand * CFCommand::createResetLogs(Drone * d)
{
	crtpLogResetRequest r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogResetRequest)), RESET_LOGS);
}

CFCommand * CFCommand::createAddLogBlock(Drone * d, int logBlockId, Array<String> variableNames)
{
	crtpLogCreateBlockV2Request r;
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

CFCommand * CFCommand::createStartLog(Drone * d, int logBlockId, int freq)
{
	crtpLogStartRequest r(logBlockId, 100 / freq);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogStartRequest)), START_LOG);
}

CFCommand * CFCommand::createStopLog(Drone * d, int logBlockId)
{
	crtpLogStopRequest r(logBlockId);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpLogStopRequest)), STOP_LOG);
}