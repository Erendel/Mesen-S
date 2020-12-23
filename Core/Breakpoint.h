#pragma once
#include "stdafx.h"

enum class CpuType : uint8_t;
enum class SnesMemoryType : uint8_t;
struct AddressInfo;
enum class BreakpointTypeFlags : uint8_t;
enum class BreakpointCategory : uint8_t;

#pragma pack(push, 1)
struct Breakpoint
{
	bool Matches(uint32_t memoryAddr, AddressInfo& info);
	bool HasBreakpointType(int arrIndex);
	string GetCondition();
	bool HasCondition();

	CpuType GetCpuType();
	bool IsEnabled();
	bool IsMarked();

	CpuType cpuType;
	SnesMemoryType memoryType;
	BreakpointTypeFlags type;
	int32_t startAddr;
	int32_t endAddr;
	bool enabled;
	bool markEvent;
	char condition[1000];
};
#pragma pack(pop)
