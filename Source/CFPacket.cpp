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
	if (crtpConsoleResponse::match(ack)) {
		type = CONSOLE;
		data = ((crtpConsoleResponse*)ack.data)->text;
	} else if (crtpLogGetInfoResponse::match(ack)) {
		// handled in batch system
		crtpLogGetInfoResponse * r = (crtpLogGetInfoResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("crc", (int)r->log_crc);
		data.getDynamicObject()->setProperty("size", r->log_len);
		data.getDynamicObject()->setProperty("maxOps", r->log_max_ops);
		data.getDynamicObject()->setProperty("maxPackets", r->log_max_packet);

	} else if (crtpLogGetItemResponse::match(ack)) {
		// handled in batch system
		crtpLogGetItemResponse * r = (crtpLogGetItemResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("name", r->text);
		data.getDynamicObject()->setProperty("id", r->type);

	} else if (crtpLogControlResponse::match(ack)) {

		crtpLogControlResponse * r = (crtpLogControlResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("result", r->result);
		data.getDynamicObject()->setProperty("command", r->command);
		data.getDynamicObject()->setProperty("byte1", r->requestByte1);

	} else if (crtpLogDataResponse::match(ack)) {

		crtpLogDataResponse * r = (crtpLogDataResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("blockId", r->blockId);
		data.getDynamicObject()->setProperty("data", r->data);
/*
		auto iter = m_logBlockCb.find(r->blockId);
		if (iter != m_logBlockCb.end()) {
			iter->second(r, result.size - 5);
		} else {
			m_logger.warning("Received unrequested data for block: " + std::to_string((int)r->blockId));
		}
		*/
	} else if (crtpParamTocGetInfoResponse::match(ack)) {
		crtpParamTocGetInfoResponse * r = (crtpParamTocGetInfoResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("crc", (int)r->crc);
		data.getDynamicObject()->setProperty("size", r->numParam);

	} else if (crtpParamTocGetItemResponse::match(ack)) {
		crtpParamTocGetItemResponse * r = (crtpParamTocGetItemResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("group", r->group);
		data.getDynamicObject()->setProperty("type", r->type);
		data.getDynamicObject()->setProperty("name", r->text);
		data.getDynamicObject()->setProperty("realOnly", r->readonly);
		data.getDynamicObject()->setProperty("length", r->length);
		data.getDynamicObject()->setProperty("sign", r->sign);

	} else if (crtpMemoryGetNumberResponse::match(ack)) {
		crtpMemoryGetNumberResponse * r = (crtpMemoryGetNumberResponse *)ack.data;
		data = r->numberOfMemories;

	} else if (crtpMemoryGetInfoResponse::match(ack)) {
		crtpMemoryGetInfoResponse * r = (crtpMemoryGetInfoResponse *)ack.data;
		data = new DynamicObject();
		data.getDynamicObject()->setProperty("address", (int64)r->memAddr);
		data.getDynamicObject()->setProperty("size", (int32)r->memSize);
		data.getDynamicObject()->setProperty("type", (int)r->memType);

	} else if (crtpParamValueResponse::match(ack)) {
		/*
		crtpParamValueResponse * r = (crtpParamValueResponse *)ack.data;
		data = new DynamicObject();
		if (drone->paramToc == nullptr)
		{
			DBG("Drone has no param toc !");
			return;
		}
		
		drone->paramToc->params[r->request.id];
		data.getDynamicObject()->setProperty("crc", (int)r->);
		data.getDynamicObject()->setProperty("group", r->group);
		data.getDynamicObject()->setProperty("type", r->type);
		data.getDynamicObject()->setProperty("name", r->text);
		data.getDynamicObject()->setProperty("realOnly", r->readonly);
		data.getDynamicObject()->setProperty("length", r->length);
		data.getDynamicObject()->setProperty("sign", r->sign);
		*/
	} else if (crtpPlatformRSSIAck::match(ack)) {
		
		/*
		crtpPlatformRSSIAck* r = (crtpPlatformRSSIAck*)ack.data;
		if (m_emptyAckCallback) {
			m_emptyAckCallback(r);
		}
		*/
	} else {
		/*
		crtp* header = (crtp*)result.data;
		m_logger.warning("Don't know ack: Port: " + std::to_string((int)header->port)
			+ " Channel: " + std::to_string((int)header->channel)
			+ " Len: " + std::to_string((int)result.size));
		// for (size_t i = 1; i < result.size; ++i) {
		//   std::cout << "    " << (int)result.data[i] << std::endl;
		// }
		queueGenericPacket(result);

		*/
	}
}

CFPacket::~CFPacket()
{
}
