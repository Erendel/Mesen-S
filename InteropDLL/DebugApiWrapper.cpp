#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/Debugger.h"
#include "../Core/TraceLogger.h"
#include "../Core/MemoryDumper.h"
#include "../Core/MemoryAccessCounter.h"
#include "../Core/Disassembler.h"
#include "../Core/DebugTypes.h"
#include "../Core/Breakpoint.h"
#include "../Core/BreakpointManager.h"
#include "../Core/PpuTools.h"
#include "../Core/CodeDataLogger.h"
#include "../Core/EventManager.h"
#include "../Core/CallstackManager.h"
#include "../Core/LabelManager.h"
#include "../Core/ScriptManager.h"
#include "../Core/Profiler.h"
#include "../Core/Assembler.h"
#include "../Core/BaseEventManager.h"

extern shared_ptr<Console> _console;
static string _logString;

shared_ptr<Debugger> GetDebugger()
{
	return _console.get() ? _console->GetDebugger() : shared_ptr<Debugger>();
}

extern "C"
{
	//Debugger wrapper
	DllExport void __stdcall InitializeDebugger()
	{
		GetDebugger();
	}

	DllExport void __stdcall ReleaseDebugger()
	{
		if (_console.get()) {
			_console->StopDebugger();
		}
	}

	DllExport bool __stdcall IsDebuggerRunning()
	{
		auto dbg = _console.get() ? _console->GetDebugger(false) : nullptr;
		return dbg.get() ? (dbg.get() != nullptr) : false;
	}

	DllExport bool __stdcall IsExecutionStopped()
	{
		auto dbg = GetDebugger();
		return dbg.get() ? dbg->IsExecutionStopped() : true;
	}
	DllExport void __stdcall ResumeExecution() { if (IsDebuggerRunning()) GetDebugger()->Run(); }
	DllExport void __stdcall Step(CpuType cpuType, uint32_t count, StepType type)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->Step(cpuType, count, type);
		}
	}

	DllExport void __stdcall RefreshDisassembly(CpuType type)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetDisassembler()->RefreshDisassembly(type);
		}
	}
	DllExport void __stdcall GetDisassemblyLineData(CpuType type, uint32_t lineIndex, CodeLineData &data)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetDisassembler()->GetLineData(type, lineIndex, data);
		}
	}
	DllExport uint32_t __stdcall GetDisassemblyLineCount(CpuType type)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetDisassembler()->GetLineCount(type) : 0;
	}
	DllExport uint32_t __stdcall GetDisassemblyLineIndex(CpuType type, uint32_t cpuAddress)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetDisassembler()->GetLineIndex(type, cpuAddress) : 0;
	}
	DllExport int32_t __stdcall SearchDisassembly(CpuType type, const char* searchString, int32_t startPosition, int32_t endPosition, bool searchBackwards)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetDisassembler()->SearchDisassembly(type, searchString, startPosition, endPosition, searchBackwards) : 0;
	}

	DllExport void __stdcall SetTraceOptions(TraceLoggerOptions options)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetTraceLogger()->SetOptions(options);
		}
	}
	DllExport void __stdcall StartTraceLogger(char* filename)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetTraceLogger()->StartLogging(filename);
		}
	}
	DllExport void __stdcall StopTraceLogger()
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetTraceLogger()->StopLogging();
		}
	}
	DllExport void __stdcall ClearTraceLog()
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetTraceLogger()->Clear();
		}
	}
	DllExport const char* GetExecutionTrace(uint32_t lineCount)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetTraceLogger()->GetExecutionTrace(lineCount) : nullptr;
	}

	DllExport void __stdcall SetBreakpoints(Breakpoint breakpoints[], uint32_t length)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetBreakpoints(breakpoints, length);
		}
	}
	DllExport void __stdcall GetBreakpoints(CpuType cpuType, Breakpoint* breakpoints, int& execs, int& reads, int& writes)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetBreakpoints(cpuType, breakpoints, execs, reads, writes);
		}
	}

	DllExport int32_t __stdcall EvaluateExpression(char* expression, CpuType cpuType, EvalResultType *resultType, bool useCache)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->EvaluateExpression(expression, cpuType, *resultType, useCache) : 0;
	}
	DllExport void __stdcall GetCallstack(CpuType cpuType, StackFrameInfo *callstackArray, uint32_t &callstackSize)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetCallstackManager(cpuType)->GetCallstack(callstackArray, callstackSize);
		}
	}
	DllExport void __stdcall GetProfilerData(CpuType cpuType, ProfiledFunction* profilerData, uint32_t& functionCount)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetCallstackManager(cpuType)->GetProfiler()->GetProfilerData(profilerData, functionCount);
		}
	}
	DllExport void __stdcall ResetProfiler(CpuType cpuType)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetCallstackManager(cpuType)->GetProfiler()->Reset();
		}
	}

	DllExport void __stdcall GetState(DebugState& state)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetState(state, false);
		}
	}
	DllExport bool __stdcall GetCpuProcFlag(ProcFlags::ProcFlags flag)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetCpuProcFlag(flag) : 0;
	}

	DllExport void __stdcall SetCpuRegister(CpuRegister reg, uint16_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetCpuRegister(reg, value);
		}
	}
	DllExport void __stdcall SetCpuProcFlag(ProcFlags::ProcFlags flag, bool set)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetCpuProcFlag(flag, set);
		}
	}
	DllExport void __stdcall SetSpcRegister(SpcRegister reg, uint16_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetSpcRegister(reg, value);
		}
	}
	DllExport void __stdcall SetNecDspRegister(NecDspRegister reg, uint16_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetNecDspRegister(reg, value);
		}
	}
	DllExport void __stdcall SetSa1Register(CpuRegister reg, uint16_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetSa1Register(reg, value);
		}
	}
	DllExport void __stdcall SetGsuRegister(GsuRegister reg, uint16_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetGsuRegister(reg, value);
		}
	}
	DllExport void __stdcall SetCx4Register(Cx4Register reg, uint32_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetCx4Register(reg, value);
		}
	}
	DllExport void __stdcall SetGameboyRegister(GbRegister reg, uint16_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetGameboyRegister(reg, value);
		}
	}

	DllExport const char* __stdcall GetDebuggerLog()
	{
		auto dbg = GetDebugger();

		_logString = dbg.get() ? dbg->GetLog() : "";
		return _logString.c_str();
	}

	DllExport void __stdcall SetMemoryState(SnesMemoryType type, uint8_t *buffer, int32_t length)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetMemoryDumper()->SetMemoryState(type, buffer, length);
		}
	}
	DllExport uint32_t __stdcall GetMemorySize(SnesMemoryType type)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetMemoryDumper()->GetMemorySize(type) : 0;
	}
	DllExport void __stdcall GetMemoryState(SnesMemoryType type, uint8_t *buffer)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetMemoryDumper()->GetMemoryState(type, buffer);
		}
	}
	DllExport uint8_t __stdcall GetMemoryValue(SnesMemoryType type, uint32_t address)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetMemoryDumper()->GetMemoryValue(type, address) : 0;
	}
	DllExport void __stdcall SetMemoryValue(SnesMemoryType type, uint32_t address, uint8_t value)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetMemoryDumper()->SetMemoryValue(type, address, value);
		}
	}
	DllExport void __stdcall SetMemoryValues(SnesMemoryType type, uint32_t address, uint8_t* data, int32_t length)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetMemoryDumper()->SetMemoryValues(type, address, data, length);
		}
	}

	DllExport AddressInfo __stdcall GetAbsoluteAddress(AddressInfo relAddress)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetAbsoluteAddress(relAddress) : AddressInfo();
	}
	DllExport AddressInfo __stdcall GetRelativeAddress(AddressInfo absAddress, CpuType cpuType)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetRelativeAddress(absAddress, cpuType) : AddressInfo();
	}

	DllExport void __stdcall SetLabel(uint32_t address, SnesMemoryType memType, char* label, char* comment)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetLabelManager()->SetLabel(address, memType, label, comment);
		}
	}
	DllExport void __stdcall ClearLabels()
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetLabelManager()->ClearLabels();
		}
	}

	DllExport void __stdcall ResetMemoryAccessCounts()
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetMemoryAccessCounter()->ResetCounts();
		}
	}
	DllExport void __stdcall GetMemoryAccessCounts(uint32_t offset, uint32_t length, SnesMemoryType memoryType, AddressCounters* counts)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetMemoryAccessCounter()->GetAccessCounts(offset, length, memoryType, counts);
		}
	}
	
	DllExport void __stdcall GetCdlData(uint32_t offset, uint32_t length, SnesMemoryType memoryType, uint8_t* cdlData)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetCdlData(offset, length, memoryType, cdlData);
		}
	}
	DllExport void __stdcall SetCdlData(CpuType cpuType, uint8_t* cdlData, uint32_t length)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SetCdlData(cpuType, cdlData, length);
		}
	}
	DllExport void __stdcall MarkBytesAs(CpuType cpuType, uint32_t start, uint32_t end, uint8_t flags)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->MarkBytesAs(cpuType, start, end, flags);
		}
	}
	
	DllExport void __stdcall GetTilemap(GetTilemapOptions options, PpuState state, uint8_t *vram, uint8_t *cgram, uint32_t *buffer)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetPpuTools()->GetTilemap(options, state, vram, cgram, buffer);
		}
	}
	DllExport void __stdcall GetTileView(GetTileViewOptions options, uint8_t *source, uint32_t srcSize, uint8_t *cgram, uint32_t *buffer)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetPpuTools()->GetTileView(options, source, srcSize, cgram, buffer);
		}
	}
	DllExport void __stdcall GetSpritePreview(GetSpritePreviewOptions options, PpuState state, uint8_t* vram, uint8_t *oamRam, uint8_t *cgram, uint32_t *buffer)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetPpuTools()->GetSpritePreview(options, state, vram, oamRam, cgram, buffer);
		}
	}
	DllExport void __stdcall SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle, CpuType cpuType)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetPpuTools()->SetViewerUpdateTiming(viewerId, scanline, cycle, cpuType);
		}
	}

	DllExport void __stdcall GetGameboyTilemap(uint8_t* vram, GbPpuState state, uint16_t offset, uint32_t* buffer)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetPpuTools()->GetGameboyTilemap(vram, state, offset, buffer);
		}
	}
	DllExport void __stdcall GetGameboySpritePreview(GetSpritePreviewOptions options, GbPpuState state, uint8_t* vram, uint8_t* oamRam, uint32_t* buffer)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetPpuTools()->GetGameboySpritePreview(options, state, vram, oamRam, buffer);
		}
	}

	DllExport void __stdcall GetDebugEvents(CpuType cpuType, DebugEventInfo *infoArray, uint32_t &maxEventCount)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetEventManager(cpuType)->GetEvents(infoArray, maxEventCount);
		}
	}
	DllExport uint32_t __stdcall GetDebugEventCount(CpuType cpuType, EventViewerDisplayOptions options)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetEventManager(cpuType)->GetEventCount(options) : 0;
	}
	DllExport void __stdcall GetEventViewerOutput(CpuType cpuType, uint32_t *buffer, uint32_t bufferSize, EventViewerDisplayOptions options)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetEventManager(cpuType)->GetDisplayBuffer(buffer, bufferSize, options);
		}
	}
	DllExport void __stdcall GetEventViewerEvent(CpuType cpuType, DebugEventInfo *evtInfo, uint16_t scanline, uint16_t cycle, EventViewerDisplayOptions options)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			*evtInfo = dbg->GetEventManager(cpuType)->GetEvent(scanline, cycle, options);
		}
	}
	DllExport uint32_t __stdcall TakeEventSnapshot(CpuType cpuType, EventViewerDisplayOptions options)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetEventManager(cpuType)->TakeEventSnapshot(options) : 0;
	}

	DllExport int32_t __stdcall LoadScript(char* name, char* content, int32_t scriptId)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetScriptManager()->LoadScript(name, content, scriptId) : 0;
	}
	DllExport void __stdcall RemoveScript(int32_t scriptId)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->GetScriptManager()->RemoveScript(scriptId);
		}
	}
	DllExport const char* __stdcall GetScriptLog(int32_t scriptId)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetScriptManager()->GetScriptLog(scriptId) : "";
	}
	//DllExport void __stdcall DebugSetScriptTimeout(uint32_t timeout) { LuaScriptingContext::SetScriptTimeout(timeout); }

	DllExport uint32_t __stdcall AssembleCode(CpuType cpuType, char* code, uint32_t startAddress, int16_t* assembledOutput)
	{
		auto dbg = GetDebugger();

		return dbg.get() ? dbg->GetAssembler(cpuType)->AssembleCode(code, startAddress, assembledOutput) : 0;
	}
	
	DllExport void __stdcall SaveRomToDisk(char* filename, bool saveIpsFile, CdlStripOption cdlStripOption)
	{
		auto dbg = GetDebugger();

		if (dbg.get()) {
			dbg->SaveRomToDisk(filename, saveIpsFile, cdlStripOption);
		}
	}
};