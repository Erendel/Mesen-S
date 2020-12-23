#include "stdafx.h"
#include "Breakpoint.h"
#include "DebugTypes.h"
#include "DebugUtilities.h"

bool Breakpoint::Matches(uint32_t memoryAddr, AddressInfo& info)
{
	if (memoryType <= DebugUtilities::GetLastCpuMemoryType() && !DebugUtilities::IsPpuMemory(info.Type))
	{
		if (startAddr == -1)
		{
			return true;
		}
		else if (endAddr == -1)
		{
			return static_cast<int32_t>(memoryAddr) == startAddr;
		}
		else
		{
			return static_cast<int32_t>(memoryAddr) >= startAddr && static_cast<int32_t>(memoryAddr) <= endAddr;
		}
	}
	else if (memoryType == info.Type)
	{
		if (startAddr == -1)
		{
			return true;
		}
		else if (endAddr == -1)
		{
			return info.Address == startAddr;
		}
		else
		{
			return info.Address >= startAddr && info.Address <= endAddr;
		}
	}

	return false;
}

bool Breakpoint::HasBreakpointType(int arrIndex)
{
	switch (arrIndex)
	{
	default:
	case 0: return ((uint8_t)type & (uint8_t)BreakpointTypeFlags::Execute) != 0;
	case 1: return ((uint8_t)type & (uint8_t)BreakpointTypeFlags::Read) != 0;
	case 2: return ((uint8_t)type & (uint8_t)BreakpointTypeFlags::Write) != 0;
	}
}

string Breakpoint::GetCondition()
{
	return condition;
}

bool Breakpoint::HasCondition()
{
	return condition[0] != 0;
}

CpuType Breakpoint::GetCpuType()
{
	return cpuType;
}

bool Breakpoint::IsEnabled()
{
	return enabled;
}

bool Breakpoint::IsMarked()
{
	return markEvent;
}
