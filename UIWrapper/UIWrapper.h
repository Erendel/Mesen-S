#pragma once

#include "../Core/DebugTypes.h"
#include "../Core/Breakpoint.h"
#include "../Core/INotificationListener.h"

typedef void (__stdcall* NotificationListenerCallback)(::ConsoleNotificationType, void*);

enum class ConsoleId : uint8_t
{
	Main = 0,
	HistoryViewer = 1
};

extern "C" {
bool __stdcall IsMesenStarted();

void __stdcall Start(const char* rom);
void __stdcall Stop();
void __stdcall CloseGui();

void __stdcall ResumeExecution();
unsigned int __stdcall GetPcAddress();
void __stdcall Step(::CpuType cpuType, int instructionCount, ::StepType type);
void __stdcall GetBreakpoints(::CpuType cpuType, ::Breakpoint* breakpoints, int& execs, int& reads, int& writes);
uint32_t __stdcall GetMemorySize(::SnesMemoryType type);
void __stdcall GetMemoryState(::SnesMemoryType type, uint8_t* buffer);
void __stdcall GetState(::DebugState& state);
bool __stdcall GetCpuProcFlag(::ProcFlags::ProcFlags flag);
void __stdcall SetCpuRegister(::CpuRegister reg, uint16_t value);

void* __stdcall RegisterNotificationCallback(NotificationListenerCallback callback);
void __stdcall UnregisterNotificationCallback(void* notificationListener);
void __stdcall Release();
void __stdcall Pause(::ConsoleId consoleId);
}
