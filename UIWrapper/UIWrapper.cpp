#include "pch.h"

#include "UIWrapper.h"

#include <msclr\marshal.h>

using namespace System::Runtime::InteropServices;
using namespace System;
using namespace System::Windows::Forms;
using namespace msclr::interop;
using namespace Mesen::GUI;

extern "C" {
	bool __stdcall IsMesenStarted() {
		return EmuApi::IsRunning();
	}

	void __stdcall Start(const char* rom) {
		CoInitializeEx(NULL, COINIT_MULTITHREADED);

		array<String^>^ arr = gcnew array<String^>(1);

		arr[0] = marshal_as<String^>(rom);

		Program::Main(arr);
	}

	void __stdcall Stop() {
		DebugApi::ResumeExecution();

		EmuApi::Stop();
		EmuApi::Release();
	}

	void __stdcall CloseGui() {
		Application::Exit();

		CoUninitialize();
	}

	void __stdcall Step(::CpuType cpuType, int instructionCount, ::StepType type) {
		DebugApi::Step((Mesen::GUI::CpuType)cpuType, instructionCount, (Mesen::GUI::StepType)type);
	}

	void __stdcall GetBreakpoints(::CpuType cpuType, ::Breakpoint* breakpoints, int& execs, int& reads, int& writes) {
		if (breakpoints != nullptr) {
			array<Mesen::GUI::Debugger::InteropBreakpoint>^ bps = gcnew array<Mesen::GUI::Debugger::InteropBreakpoint>(execs + reads + writes);

			DebugApi::GetBreakpoints((Mesen::GUI::CpuType)cpuType, bps, execs, reads, writes);

			for (auto i = 0; i < execs + reads + writes; ++i)
			{
				breakpoints[i].enabled = bps[i].Enabled;
				breakpoints[i].startAddr = bps[i].StartAddress;
				breakpoints[i].endAddr = bps[i].EndAddress;
				breakpoints[i].cpuType = (::CpuType)bps[i].CpuType;
				breakpoints[i].markEvent = bps[i].MarkEvent;
				breakpoints[i].memoryType = (::SnesMemoryType)bps[i].MemoryType;
				breakpoints[i].type = (::BreakpointTypeFlags)bps[i].Type;

				pin_ptr<unsigned char> arr_start = &bps[i].Condition[0];
				memcpy(breakpoints[i].condition, arr_start, sizeof(breakpoints[i].condition));
			}
		}
		else {
			DebugApi::GetBreakpoints((Mesen::GUI::CpuType)cpuType, nullptr, execs, reads, writes);
		}
	}

	void __stdcall SetBreakpoints(::Breakpoint breakpoints[], uint32_t length)
	{
		array<Mesen::GUI::Debugger::InteropBreakpoint>^ bps = gcnew array<Mesen::GUI::Debugger::InteropBreakpoint>(length);

		for (uint32_t i = 0; i < length; ++i)
		{
			bps[i].Enabled = breakpoints[i].enabled;
			bps[i].StartAddress = breakpoints[i].startAddr;
			bps[i].EndAddress = breakpoints[i].endAddr;
			bps[i].CpuType = (Mesen::GUI::CpuType)breakpoints[i].cpuType;
			bps[i].MarkEvent = breakpoints[i].markEvent;
			bps[i].MemoryType = (Mesen::GUI::SnesMemoryType)breakpoints[i].memoryType;
			bps[i].Type = (Mesen::GUI::Debugger::BreakpointTypeFlags)breakpoints[i].type;
			bps[i].Condition = gcnew array<unsigned char>(sizeof(breakpoints[i].condition));
			pin_ptr<unsigned char> ptr = &bps[i].Condition[0];
			memcpy(ptr, breakpoints[i].condition, sizeof(breakpoints[i].condition));
		}
		
		DebugApi::SetBreakpoints(bps, length);
	}

	unsigned int __stdcall GetPcAddress() {
		return DebugApi::GetPcAddress();
	}

	::AddressInfo __stdcall GetAbsoluteAddress(::AddressInfo relAddress)
	{
		auto result = DebugApi::GetAbsoluteAddress(Mesen::GUI::AddressInfo{relAddress.Address, (Mesen::GUI::SnesMemoryType)relAddress.Type});
		return {result.Address, (::SnesMemoryType)result.Type};
	}

	::AddressInfo __stdcall GetRelativeAddress(::AddressInfo absAddress, ::CpuType cpuType)
	{
		auto result = DebugApi::GetRelativeAddress(Mesen::GUI::AddressInfo{absAddress.Address, (Mesen::GUI::SnesMemoryType)absAddress.Type}, (Mesen::GUI::CpuType)cpuType);
		return {result.Address, (::SnesMemoryType)result.Type};
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
		Mesen::GUI::DebugState dbgState = DebugApi::GetState();
		IntPtr pnt = Marshal::AllocHGlobal(Marshal::SizeOf(dbgState)); 
		Marshal::StructureToPtr(dbgState, pnt, false);
		memcpy(&state, pnt.ToPointer(), sizeof(state));
		Marshal::FreeHGlobal(pnt);
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