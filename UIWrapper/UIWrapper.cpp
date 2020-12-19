#include "pch.h"

#include "UIWrapper.h"

#include <msclr\marshal.h>

using namespace System::Runtime::InteropServices;
using namespace System;
using namespace msclr::interop;
using namespace Mesen::GUI;

extern "C" {

	bool __stdcall IsMesenStarted() {
		return EmuApi::IsRunning();
	}

	void __stdcall Start(const char* rom) {
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		//OleInitialize(NULL);

		array<String^>^ arr = gcnew array<String^>(1);

		arr[0] = marshal_as<String^>(rom);

		Program::Main(arr);

		//OleUninitialize();
		CoUninitialize();
	}

	void __stdcall Step(::CpuType cpuType, int instructionCount, ::StepType type) {
		DebugApi::Step((Mesen::GUI::CpuType)cpuType, instructionCount, (Mesen::GUI::StepType)type);
	}

	void __stdcall GetBreakpoints(::CpuType cpuType, ::Breakpoint* breakpoints, int& execs, int& reads, int& writes) {
		if (breakpoints != nullptr) {
			array<Mesen::GUI::Debugger::Breakpoint^>^ bps = gcnew array<Mesen::GUI::Debugger::Breakpoint^>(execs + reads + writes);
			DebugApi::GetBreakpoints((Mesen::GUI::CpuType)cpuType, bps, execs, reads, writes);
			pin_ptr<Mesen::GUI::Debugger::Breakpoint^> bps_start = &bps[0];
			memcpy(breakpoints, bps_start, execs + reads + writes);
		}
		else {
			DebugApi::GetBreakpoints((Mesen::GUI::CpuType)cpuType, nullptr, execs, reads, writes);
		}
	}

	unsigned int __stdcall GetPcAddress() {
		return DebugApi::GetPcAddress();
	}

	void* __stdcall RegisterNotificationCallback(NotificationListenerCallback callback) {
		auto ptr = Marshal::GetDelegateForFunctionPointer<NotificationListener::NotificationCallback^>(IntPtr(callback));
		return (void*)EmuApi::RegisterNotificationCallback(ptr);
	}

	void __stdcall UnregisterNotificationCallback(void* notificationListener) {
		EmuApi::UnregisterNotificationCallback(IntPtr(notificationListener));
	}

	uint32_t __stdcall GetMemorySize(::SnesMemoryType type) {
		return DebugApi::GetMemorySize((Mesen::GUI::SnesMemoryType)type);
	}

	void __stdcall GetMemoryState(::SnesMemoryType type, uint8_t* buffer) {
		auto size = GetMemorySize(type);
		auto arr = gcnew array<unsigned char>(size);

		DebugApi::GetMemoryStateWrapper((Mesen::GUI::SnesMemoryType)type, arr);

		pin_ptr<unsigned char> arr_start = &arr[0];
		memcpy(buffer, arr_start, size);
	}

	void __stdcall GetState(::DebugState& state) {
		DebugApi::GetStateWrapper((Mesen::GUI::DebugState%)state);
	}

	bool __stdcall GetCpuProcFlag(::ProcFlags::ProcFlags flag) {
		return DebugApi::GetCpuProcFlag((Mesen::GUI::ProcFlags)flag);
	}

	void __stdcall SetCpuRegister(::CpuRegister reg, uint16_t value) {
		DebugApi::SetCpuRegister((Mesen::GUI::CpuRegister)reg, value);
	}

	void __stdcall Release() {
		EmuApi::Release();
	}

	void __stdcall ResumeExecution() {
		DebugApi::ResumeExecution();
	}

	void __stdcall Pause(::ConsoleId consoleId) {
		EmuApi::Pause((Mesen::GUI::EmuApi::ConsoleId)consoleId);
	}
}
