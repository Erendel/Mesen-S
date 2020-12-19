#pragma once
#include "stdafx.h"
#include "NetMessage.h"

class SelectControllerMessage : public NetMessage
{
private:
	uint8_t _portNumber;

protected:
	void Serialize(Serializer& s) override
	{
		s.Stream(_portNumber);
	}

public:
	SelectControllerMessage(void* buffer, uint32_t length) : NetMessage(buffer, length)
	{
	}

	SelectControllerMessage(uint8_t port) : NetMessage(MessageType::SelectController)
	{
		_portNumber = port;
	}

	uint8_t GetPortNumber()
	{
		return _portNumber;
	}
};
