/*
  ==============================================================================

	CFCommand.cpp
	Created: 19 Jun 2018 9:04:27am
	Author:  Ben

  ==============================================================================
*/

#include "CFCommand.h"
#include "CFDrone.h"
#include "CFParam.h"

CFCommand::CFCommand(CFDrone * drone, Array<uint8> _data, Type type) : 
	drone(drone), 
	type(type)
{
	data.addArray(_data);
}

CFCommand * CFCommand::createPing(CFDrone * d) {
	return new CFCommand(d, Array<uint8>(0xFF), PING);
}
CFCommand * CFCommand::createSetPoint(CFDrone * d, float yaw, float pitch, float roll, float thrust) {
	crtpSetpointRequest r(roll, pitch, yaw, thrust);
	return new CFCommand(d, Array<uint8>((uint8 *)&r, sizeof(crtpSetpointRequest)), SETPOINT);
}
CFCommand * CFCommand::createVelocity(CFDrone * d, Vector3D<float> vel, float yaw) {
	crtpVelocitySetpointRequest r(vel.x, vel.y, vel.z, yaw);
	return new CFCommand(d, Array<uint8>((uint8 *)&r), VELOCITY);
}
CFCommand * CFCommand::createPosition(CFDrone * d, Vector3D<float> pos, float yaw) {
	crtpPositionSetpointRequest r(pos.x, pos.y, pos.z, yaw);
	return new CFCommand(d, Array<uint8>((uint8 *)&r), POSITION);
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
	case CFParam::Type::Uint8: { crtpParamWriteRequest<uint8_t> r(p->definition.id, (uint8_t)(int)value); data = Array<uint8>((uint8 *)&r); } break;

	case CFParam::Type::Int8: { crtpParamWriteRequest<int8_t> r(p->definition.id, (int8_t)(int)value); data = Array<uint8>((uint8 *)&r); }	break;
	case CFParam::Type::Uint16: { crtpParamWriteRequest<uint16_t> r(p->definition.id, (uint16_t)(int)value); data = Array<uint8>((uint8 *)&r);  }break;
	case CFParam::Type::Int16: { crtpParamWriteRequest<int16_t> r(p->definition.id, (int16_t)(int)value); data = Array<uint8>((uint8 *)&r);  }break;
	case CFParam::Type::Uint32: { crtpParamWriteRequest<uint32_t> r(p->definition.id, (uint32_t)(int)value); data = Array<uint8>((uint8 *)&r); }break;
	case CFParam::Type::Int32: { crtpParamWriteRequest<int32_t> r(p->definition.id, value); data = Array<uint8>((uint8 *)&r);  }break;
	case CFParam::Type::Float: { crtpParamWriteRequest<float> r(p->definition.id, value); data = Array<uint8>((uint8 *)&r);  }break;
	default: DBG("Type not handled : " << (int)p->definition.type); return nullptr; break;
	}

	return new CFCommand(d, data, SET_PARAM);
}

CFCommand * CFCommand::createGetParam(CFDrone * d, StringRef name)
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
	return new CFCommand(d, Array<uint8>((uint8 *)&r), SET_PARAM);
}

CFCommand * CFCommand::createRequestParamToc(CFDrone * d)
{
	crtpParamTocGetInfoRequest r;
	return new CFCommand(d, Array<uint8>((uint8 *)&r), SET_PARAM);
}
