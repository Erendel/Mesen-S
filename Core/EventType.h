#pragma once

#pragma pack(push, 1)
enum class EventType : uint8_t
{
	Nmi,
	Irq,
	StartFrame,
	EndFrame,
	Reset,
	ScriptEnded,
	InputPolled,
	StateLoaded,
	StateSaved,
	GbStartFrame,
	GbEndFrame,
	EventTypeSize
};
#pragma pack(pop)
