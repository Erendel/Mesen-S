#include "stdafx.h"
#include "BreakpointManager.h"
#include "DebugTypes.h"
#include "Debugger.h"
#include "Breakpoint.h"
#include "DebugUtilities.h"
#include "ExpressionEvaluator.h"
#include "EventManager.h"

BreakpointManager::BreakpointManager(Debugger* debugger, CpuType cpuType, IEventManager* eventManager)
{
	_debugger = debugger;
	_cpuType = cpuType;
	_hasBreakpoint = false;
	_eventManager = eventManager ? eventManager : debugger->GetEventManager(cpuType).get();
}

void BreakpointManager::SetBreakpoints(Breakpoint breakpoints[], uint32_t count)
{
	_hasBreakpoint = false;
	for (int i = 0; i < BreakpointManager::BreakpointTypeCount; i++)
	{
		_breakpoints[i].clear();
		_rpnList[i].clear();
		_hasBreakpointType[i] = false;
	}

	_bpExpEval.reset(new ExpressionEvaluator(_debugger, _cpuType));

	for (uint32_t j = 0; j < count; j++)
	{
		Breakpoint& bp = breakpoints[j];
		for (int i = 0; i < BreakpointManager::BreakpointTypeCount; i++)
		{
			if ((bp.IsMarked() || bp.IsEnabled()) && bp.HasBreakpointType(i))
			{
				CpuType cpuType = bp.GetCpuType();
				if (_cpuType != cpuType)
				{
					continue;
				}

				int curIndex = _breakpoints[i].size();
				_breakpoints[i].insert(std::make_pair(curIndex, bp));

				if (bp.HasCondition())
				{
					bool success = true;
					ExpressionData data = _bpExpEval->GetRpnList(bp.GetCondition(), success);
					_rpnList[i].insert(std::make_pair(curIndex, success ? data : ExpressionData()));
				}
				else
				{
					_rpnList[i].insert(std::make_pair(curIndex, ExpressionData()));
				}

				_hasBreakpoint = true;
				_hasBreakpointType[i] = true;
			}
		}
	}
}

void BreakpointManager::GetBreakpoints(Breakpoint* breakpoints, int& execs, int& reads, int& writes)
{
	execs = _breakpoints[0].size();
	reads = _breakpoints[1].size();
	writes = _breakpoints[2].size();

	if (breakpoints == NULL)
	{
		return;
	}

	int offset = 0;
	for (auto it = _breakpoints[0].cbegin(); it != _breakpoints[0].cend(); it++)
	{
		breakpoints[offset++] = it->second;
	}

	for (auto it = _breakpoints[1].cbegin(); it != _breakpoints[1].cend(); it++)
	{
		breakpoints[offset++] = it->second;
	}

	for (auto it = _breakpoints[2].cbegin(); it != _breakpoints[2].cend(); it++)
	{
		breakpoints[offset++] = it->second;
	}
}

int BreakpointManager::GetBreakpointTypeArrayIndex(MemoryOperationType type)
{
	switch (type)
	{
	default:
		return -1;
	//case MemoryOperationType::ExecOperand:
	case MemoryOperationType::ExecOpCode:
		return 0;

	case MemoryOperationType::DmaRead:
	case MemoryOperationType::Read:
		return 1;

	case MemoryOperationType::DmaWrite:
	case MemoryOperationType::Write:
		return 2;
	}
}

int BreakpointManager::InternalCheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo& address)
{
	int arrIndex = GetBreakpointTypeArrayIndex(operationInfo.Type);

	if (arrIndex == -1 || !_hasBreakpointType[arrIndex])
	{
		return -1;
	}

	DebugState state;
	_debugger->GetState(state, false);
	EvalResultType resultType;
	unordered_map<int, Breakpoint>& breakpoints = _breakpoints[arrIndex];
	for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it)
	{
		if (it->second.Matches(operationInfo.Address, address))
		{
			if (!it->second.HasCondition() || _bpExpEval->Evaluate(_rpnList[arrIndex][it->first], state, resultType,
			                                                       operationInfo))
			{
				if (it->second.IsMarked())
				{
					_eventManager->AddEvent(DebugEventType::Breakpoint, operationInfo, it->first);
				}
				if (it->second.IsEnabled())
				{
					return it->first;
				}
			}
		}
	}

	return -1;
}
