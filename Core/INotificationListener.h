#pragma once
#include "stdafx.h"

#pragma pack(push, 1)
enum class ConsoleNotificationType : uint8_t
{
	GameLoaded = 0,
	StateLoaded = 1,
	GameReset = 2,
	GamePaused = 3,
	GameResumed = 4,
	CodeBreak = 6,
	PpuFrameDone = 7,
	ResolutionChanged = 8,
	ConfigChanged = 9,
	ExecuteShortcut = 10,
	EmulationStopped = 11,
	BeforeEmulationStop = 12,
	ViewerRefresh = 13,
	EventViewerRefresh = 14,
	MissingFirmware = 15,
	BeforeGameUnload = 16,
	CheatsChanged = 17
};
#pragma pack(pop)

class INotificationListener
{
public:
	virtual void ProcessNotification(ConsoleNotificationType type, void* parameter) = 0;
};
