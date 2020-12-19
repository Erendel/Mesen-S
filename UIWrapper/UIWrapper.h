#pragma once

#include "../Core/DebugTypes.h"
#include "../Core/Breakpoint.h"
#include "../Core/INotificationListener.h"

#ifdef STATIC_API
#    define LIBRARY_API
#else
	#ifdef LIBRARY_EXPORTS
	#    define LIBRARY_API __declspec(dllexport)
	#else
	#    define LIBRARY_API __declspec(dllimport)
	#endif
#endif

#    define LIBRARY_API


typedef void(__stdcall* NotificationListenerCallback)(::ConsoleNotificationType, void*);

enum class ConsoleId
{
	Main = 0,
	HistoryViewer = 1
};

extern "C" {

	LIBRARY_API bool __stdcall IsMesenStarted();

	LIBRARY_API void __stdcall Start(const char* rom);
	LIBRARY_API void __stdcall ResumeExecution();
	LIBRARY_API unsigned int __stdcall GetPcAddress();
	LIBRARY_API void __stdcall Step(::CpuType cpuType, int instructionCount, ::StepType type);
	LIBRARY_API void __stdcall GetBreakpoints(::CpuType cpuType, ::Breakpoint* breakpoints, int& execs, int& reads, int& writes);
	LIBRARY_API uint32_t __stdcall GetMemorySize(::SnesMemoryType type);
	LIBRARY_API void __stdcall GetMemoryState(::SnesMemoryType type, uint8_t* buffer);
	LIBRARY_API void __stdcall GetState(::DebugState& state);
	LIBRARY_API bool __stdcall GetCpuProcFlag(::ProcFlags::ProcFlags flag);
	LIBRARY_API void __stdcall SetCpuRegister(::CpuRegister reg, uint16_t value);

	LIBRARY_API void* __stdcall RegisterNotificationCallback(NotificationListenerCallback callback);
	LIBRARY_API void __stdcall UnregisterNotificationCallback(void* notificationListener);
	LIBRARY_API void __stdcall Release();
	LIBRARY_API void __stdcall Pause(::ConsoleId consoleId);

}