#pragma once
#include "stdafx.h"
#include <deque>
#include "BaseControlDevice.h"

class Console;

class RewindData
{
private:
	vector<uint8_t> SaveStateData;

public:
	std::deque<ControlDeviceState> InputLogs[BaseControlDevice::PortCount];
	int32_t FrameCount = 0;
	bool EndOfSegment = false;

	void GetStateData(stringstream &stateData);

	void LoadState(shared_ptr<Console> &console);
	void SaveState(shared_ptr<Console> &console);
};
