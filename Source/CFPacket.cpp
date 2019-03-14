/*
  ==============================================================================

	CFPacket.cpp
	Created: 19 Jun 2018 1:55:30pm
	Author:  Ben

  ==============================================================================
*/

#include "CFPacket.h"
#include "CFParam.h"
#include "CFDrone.h" 

CFPacket::CFPacket(CFDrone * drone, const ITransport::Ack & ack)
{
	hasSafeLink = memcmp(ack.data, Crazyradio::safeLinkPacket, 3) == 0;

	if (crtpConsoleResponse::match(ack)) {
		type = CONSOLE;
		const char * t = ((crtpConsoleResponse*)ack.data)->text;
		if (CharPointer_ASCII::isValidString(t, std::numeric_limits<int>::max())) data = String(t);
		else data = "[Corrupt data]";
	} else if (crtpLogGetInfoResponse::match(ack)) {
		type = LOG_TOC_INFO;
		crtpLogGetInfoResponse * r = (crtpLogGetInfoResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("crc", (int)r->log_crc);
		data.getDynamicObject()->setProperty("size", r->log_len);
		data.getDynamicObject()->setProperty("maxOps", r->log_max_ops);
		data.getDynamicObject()->setProperty("maxPackets", r->log_max_packet);

	} else if (crtpLogGetItemResponse::match(ack)) {
		type = LOG_TOC_ITEM;
		crtpLogGetItemResponse * r = (crtpLogGetItemResponse *)ack.data;
		data = new DynamicObject();
		String groupName = String(&r->text[0]);
		String variableName = String(&r->text[groupName.length() + 1]);
		data.getDynamicObject()->setProperty("id", r->request.id);
		data.getDynamicObject()->setProperty("group", groupName);
		data.getDynamicObject()->setProperty("name", variableName);
		data.getDynamicObject()->setProperty("type", r->type);

	} else if (crtpLogControlResponse::match(ack)) {
		type = LOG_CONTROL;
		crtpLogControlResponse * r = (crtpLogControlResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("result", r->result);
		data.getDynamicObject()->setProperty("command", r->command);
		data.getDynamicObject()->setProperty("byte1", r->requestByte1);

	} else if (crtpLogDataResponse::match(ack)) {
		type = LOG_DATA;
		crtpLogDataResponse * r = (crtpLogDataResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("blockId", r->blockId);
		data.getDynamicObject()->setProperty("data",var(r->data,26));

		/*
				auto iter = m_logBlockCb.find(r->blockId);
				if (iter != m_logBlockCb.end()) {
					iter->second(r, result.size - 5);
				} else {
					m_logger.warning("Received unrequested data for block: " + std::to_string((int)r->blockId));
				}
				*/
	} else if (crtpParamTocGetInfoResponse::match(ack)) {
		type = PARAM_TOC_INFO;
		crtpParamTocGetInfoResponse * r = (crtpParamTocGetInfoResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("crc", (int)r->crc);
		data.getDynamicObject()->setProperty("size", r->numParam);

	} else if (crtpParamTocGetItemResponse::match(ack)) {
		type = PARAM_TOC_ITEM;
		crtpParamTocGetItemResponse * r = (crtpParamTocGetItemResponse *)ack.data;
		data = new DynamicObject();
		String groupName = String(&r->text[0]);
		String paramName = String(&r->text[groupName.length() + 1]);
		data.getDynamicObject()->setProperty("id", r->request.id);
		data.getDynamicObject()->setProperty("group", groupName);
		data.getDynamicObject()->setProperty("type", (r->length | r->type << 2 | r->sign << 3));
		data.getDynamicObject()->setProperty("name", paramName);
		data.getDynamicObject()->setProperty("realOnly", r->readonly);
		data.getDynamicObject()->setProperty("length", r->length);
		data.getDynamicObject()->setProperty("sign", r->sign);

	}  else if (crtpParamValueResponse::match(ack)) {
		type = PARAM_VALUE;

		crtpParamValueResponse * r = (crtpParamValueResponse *)ack.data;
		data = new DynamicObject();
		if (drone->paramToc == nullptr)
		{
			DBG("Drone has no param toc !");
			return;
		}

		CFParam * p = drone->paramToc->params[r->request.id];
		if (p == nullptr)
		{
			DBG("TOC has not param with id : " << r->request.id);
			return;
		}

		data.getDynamicObject()->setProperty("id", r->request.id);
		data.getDynamicObject()->setProperty("name", p->definition.name);

		var value = var();
		switch (p->definition.type)
		{
		case CFParam::Uint8:	value = r->valueUint8; break;
		case CFParam::Int8:		value = r->valueInt8; break;
		case CFParam::Uint16:	value = r->valueUint16; break;
		case CFParam::Int16:	value = r->valueInt16; break;
		case CFParam::Uint32:	value = (int)r->valueUint32; break;
		case CFParam::Int32:	value = r->valueInt32; break;
		case CFParam::Float:	value = r->valueFloat; break;
		}

		data.getDynamicObject()->setProperty("value", value);

	} else if (crtpPlatformRSSIAck::match(ack)) {
		type = RSSI_ACK;
		crtpPlatformRSSIAck * r = (crtpPlatformRSSIAck *)ack.data;
		data = r->rssi;
	} else if (crtpLppShortPacketResponse::match(ack)) {
		type = LPP_SHORT_PACKET;
		crtpLppShortPacketResponse * r = (crtpLppShortPacketResponse *)ack.data;
		data = var();
		for (int i = 0; i < 12; i++) data.append(r->rest[i]);

	} else if (crtpMemoryGetNumberResponse::match(ack)) {
		type = MEMORY_NUMBER;
		crtpMemoryGetNumberResponse * r = (crtpMemoryGetNumberResponse *)ack.data;
		data = r->numberOfMemories;
	} else if (crtpMemoryGetInfoResponse::match(ack)) {
		type = MEMORY_INFO;
		crtpMemoryGetInfoResponse * r = (crtpMemoryGetInfoResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("address", (int64)r->memAddr);
		data.getDynamicObject()->setProperty("size", (int)r->memSize);
		data.getDynamicObject()->setProperty("type", (int)r->memType);
	} else if (crtpMemoryReadResponse::match(ack)) {
		type = MEMORY_READ;
		crtpMemoryReadResponse * r = (crtpMemoryReadResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("memoryId", r->memId);
		data.getDynamicObject()->setProperty("address", (int)r->memAddr);
		data.getDynamicObject()->setProperty("status", r->status);
		var dataData;
		for (int i = 0; i < 24; i++) dataData.append(r->data[i]);
		data.getDynamicObject()->setProperty("data", dataData);
	}else if(crtpMemoryWriteResponse::match(ack)) {
		type = MEMORY_WRITE;
		crtpMemoryWriteResponse * r = (crtpMemoryWriteResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("memoryId", r->memId);
		data.getDynamicObject()->setProperty("address", (int)r->memAddr);
		data.getDynamicObject()->setProperty("status", r->status);
	} else
	{
		type = UNKNOWN;
		
		crtp* header = (crtp*)ack.data;
		DBG("Unknown packet\nPort: " << (int)header->port << "\nChannel: " << (int)header->channel << "\nLen: " << (int)ack.size);
		// for (size_t i = 1; i < result.size; ++i) {
		//   std::cout << "    " << (int)result.data[i] << std::endl;
		// }
		//queueGenericPacket(result);
	}
}

CFPacket::~CFPacket()
{
}
