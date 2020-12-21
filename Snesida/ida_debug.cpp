#include <ida.hpp>
#include <dbg.hpp>
#include <deque>
#include <mutex>

#include "ida_plugin.h"
#include "../UIWrapper/UIWrapper.h"

static qthread_t mesen_th = nullptr;
static void* listener = nullptr;
static DebugState lastState = {};
static bool emulation_stopped = true;

static std::deque<debug_event_t> debug_events;

#define RC_CPU (1 << 0)
#define RC_PPU (1 << 1)

enum class SNES_REGS : uint8_t
{
	SR_A,
	SR_X,
	SR_Y,
	SR_SP,
	SR_D,
	SR_DBR,
	SR_PC,
	SR_PS,
	SR_A8,
	SR_I8,
	SR_EM
};

static register_info_t registers[] = {
	{"A", 0, RC_CPU, dt_word, NULL, 0},
	{"X", 0, RC_CPU, dt_word, NULL, 0},
	{"Y", 0, RC_CPU, dt_word, NULL, 0},

	{"S", REGISTER_SP, RC_CPU, dt_word, NULL, 0},

	{"D", 0, RC_CPU, dt_word, NULL, 0},
	{"DB", 0, RC_CPU, dt_byte, NULL, 0},

	{"PC", REGISTER_IP | REGISTER_ADDRESS | REGISTER_READONLY, RC_CPU, dt_dword, NULL, 0},
	{"PS", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0},

	{"A8", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0}, // A is 8 bit
	{"I8", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0}, // are X/Y 8 bits
	{"EM", REGISTER_READONLY, RC_CPU, dt_byte, NULL, 0}, // is emulation mode
};

static const char* register_classes[] = {
	"General Registers",
	NULL
};

static void processNotification(ConsoleNotificationType type, void* parameter)
{
	debug_event_t ev = {};

	ev.pid = 1;
	ev.tid = 1;
	ev.handled = true;

	switch (type)
	{
	case ConsoleNotificationType::CodeBreak:
		{
			BreakEvent* evt = static_cast<BreakEvent*>(parameter);

			switch (evt->Source)
			{
			case BreakSource::CpuStep:
			case BreakSource::PpuStep:
				return; // do not handle steps here
			case BreakSource::Unspecified:
				{
					// first dummy event for debugger
					ev.ea = GetPcAddress();
					ev.set_eid(PROCESS_SUSPENDED);

					debug_events.push_back(ev);
				}
				break;
			default:
				{
					ev.ea = evt->Operation.Address;
					ev.set_eid(BREAKPOINT);
					ev.set_bpt().hea = ev.set_bpt().kea = ev.ea;

					debug_events.push_back(ev);
				}
				break;
			}
		}
		break;
	case ConsoleNotificationType::GameLoaded:
		{
			ev.ea = BADADDR;
			ev.set_eid(PROCESS_STARTED);

			ev.set_modinfo(PROCESS_STARTED).name.sprnt("Mesen-S");
			ev.set_modinfo(PROCESS_STARTED).base = 0;
			ev.set_modinfo(PROCESS_STARTED).size = 0;
			ev.set_modinfo(PROCESS_STARTED).rebase_to = BADADDR;

			debug_events.push_back(ev);

			Step(CpuType::Cpu, 1, StepType::Step);
		}
		break;
	case ConsoleNotificationType::GamePaused:
		{
			ev.ea = GetPcAddress();
			ev.set_eid(PROCESS_SUSPENDED);

			debug_events.push_back(ev);
		}
		break;
	case ConsoleNotificationType::EmulationStopped:
		{
			emulation_stopped = true;

			ev.set_exit_code(PROCESS_EXITED, 0);

			debug_events.push_back(ev);
		}
		break;
	case ConsoleNotificationType::PpuFrameDone:
	case ConsoleNotificationType::CheatsChanged:
	case ConsoleNotificationType::ResolutionChanged:
	case ConsoleNotificationType::EventViewerRefresh:
		break;
		//default: {
		// msg("%d\n", type);
		//} break;
	}
}

static void terminate_gui()
{
	if (mesen_th != NULL)
	{
		qthread_join(mesen_th);
		qthread_free(mesen_th);
		qthread_kill(mesen_th);
		mesen_th = NULL;
	}
}

static bool init_debugger()
{
	return true;
}

static bool release_debugger()
{
	terminate_gui();

	return true;
}

static bool register_events_callback()
{
	// move debug events to shared
	listener = RegisterNotificationCallback(processNotification);

	debug_events.clear();

	return true;
}

static bool unregister_events_callback()
{
	// move debug events to shared
	UnregisterNotificationCallback(listener);

	return true;
}

static bool update_bpts(int* nbpts, update_bpt_info_t* bpts, int nadd, int ndel, qstring* errbuf)
{
	// move breakpoints list to shared
	int execs = 0, reads = 0, writes = 0;
	GetBreakpoints(CpuType::Cpu, nullptr, execs, reads, writes); // set bpts to null to get counts

	int total = execs + reads + writes;

	Breakpoint* oldBpts = (Breakpoint*)malloc(total);
	if (oldBpts == nullptr)
	{
		return false;
	}

	GetBreakpoints(CpuType::Cpu, oldBpts, execs, reads, writes);

	total += nadd - ndel;

	Breakpoint* newBpts = (Breakpoint*)malloc(total);

	if (newBpts == nullptr)
	{
		free(oldBpts);
		return false;
	}

	int newCount = 0;

	for (int j = 0; j < execs + reads + writes; ++j)
	{
		for (int i = 0; i < ndel; ++i)
		{
			int offset = 0;

			switch (bpts[nadd + i].type)
			{
			case BPT_EXEC:
				{
					offset = 0;
				}
				break;
			case BPT_READ:
				{
					offset = execs;
				}
				break;
			case BPT_WRITE:
				{
					offset = execs + reads;
				}
				break;
			default:
				continue;
			}

			Breakpoint* typeBpts = &oldBpts[offset];

			if (static_cast<int32_t>(bpts[nadd + i].ea) >= typeBpts[j].startAddr && (static_cast<int32_t>(bpts[nadd + i].ea
			) + bpts[nadd + i].size <= typeBpts[j].endAddr))
			{
				continue;
			}
		}

		memcpy(&newBpts[newCount++], &oldBpts[j], sizeof(Breakpoint));
		*nbpts += 1;
	}

	for (int i = 0; i < nadd; ++i)
	{
		auto type = static_cast<BreakpointTypeFlags>(0);
		int count = 0;

		if (bpts[i].type & BPT_SOFT) {
			type = BreakpointTypeFlags::ExecuteReadWrite;
		}
		else if (bpts[i].type & BPT_READ) {
			type = BreakpointTypeFlags::Read;
		}
		if (bpts[i].type & BPT_WRITE) {
			type = static_cast<BreakpointTypeFlags>(static_cast<int>(type) | static_cast<int>(BreakpointTypeFlags::Write));
		}
		if (bpts[i].type & BPT_EXEC) {
			type = static_cast<BreakpointTypeFlags>(static_cast<int>(type) | static_cast<int>(BreakpointTypeFlags::Execute));
		}

		Breakpoint newBp = {
			CpuType::Cpu,
			SnesMemoryType::CpuMemory,
			type,
			static_cast<int32_t>(bpts[i].ea),
			static_cast<int32_t>(bpts[i].ea + bpts[i].size),
			true,
			true,
			""
		};

		memcpy(&newBpts[newCount++], &newBp, sizeof(Breakpoint));
		*nbpts += 1;
	}

	free(oldBpts);
	free(newBpts);

	return true;
}

static bool read_memory(ea_t ea, void* buffer, size_t size, qstring* errbuf)
{
	//
	uint32_t memSize = GetMemorySize(SnesMemoryType::CpuMemory);
	uint8_t* mem = (uint8_t*)malloc(memSize);

	if (mem == nullptr)
	{
		return false;
	}

	GetMemoryState(SnesMemoryType::CpuMemory, mem);

	memcpy(buffer, &mem[ea], size);
	free(mem);

	return true;
}

static bool read_registers(thid_t tid, int clsmask, regval_t* values, qstring* errbuf)
{
	GetState(lastState);

	values[static_cast<int>(SNES_REGS::SR_A)].ival = lastState.Cpu.A;
	values[static_cast<int>(SNES_REGS::SR_X)].ival = lastState.Cpu.X;
	values[static_cast<int>(SNES_REGS::SR_Y)].ival = lastState.Cpu.Y;

	values[static_cast<int>(SNES_REGS::SR_SP)].ival = lastState.Cpu.SP;
	values[static_cast<int>(SNES_REGS::SR_D)].ival = lastState.Cpu.D;
	values[static_cast<int>(SNES_REGS::SR_DBR)].ival = lastState.Cpu.DBR;

	values[static_cast<int>(SNES_REGS::SR_PC)].ival = (lastState.Cpu.K << 16) | lastState.Cpu.PC;
	values[static_cast<int>(SNES_REGS::SR_PS)].ival = lastState.Cpu.PS;

	values[static_cast<int>(SNES_REGS::SR_A8)].ival = GetCpuProcFlag(ProcFlags::MemoryMode8);
	values[static_cast<int>(SNES_REGS::SR_I8)].ival = GetCpuProcFlag(ProcFlags::IndexMode8);
	values[static_cast<int>(SNES_REGS::SR_EM)].ival = lastState.Cpu.EmulationMode ? 1 : 0;

	return true;
}

static bool write_register(thid_t tid, int regidx, const regval_t* value, qstring* errbuf)
{
	CpuRegister cpuReg0 = CpuRegister::CpuRegPC, cpuReg1 = CpuRegister::CpuRegK;

	switch (static_cast<SNES_REGS>(regidx))
	{
	case SNES_REGS::SR_A: cpuReg0 = CpuRegister::CpuRegA;
		break;
	case SNES_REGS::SR_X: cpuReg0 = CpuRegister::CpuRegX;
		break;
	case SNES_REGS::SR_Y: cpuReg0 = CpuRegister::CpuRegY;
		break;
	case SNES_REGS::SR_SP: cpuReg0 = CpuRegister::CpuRegSP;
		break;
	case SNES_REGS::SR_D: cpuReg0 = CpuRegister::CpuRegD;
		break;
	case SNES_REGS::SR_DBR: cpuReg0 = CpuRegister::CpuRegDBR;
		break;
	case SNES_REGS::SR_PC: cpuReg0 = CpuRegister::CpuRegPC;
		break;
	default:
		return false;
	}

	if (cpuReg0 != CpuRegister::CpuRegPC)
	{
		SetCpuRegister(cpuReg0, static_cast<uint16_t>(value->ival));
	}
	else
	{
		SetCpuRegister(cpuReg1, static_cast<uint8_t>(value->ival >> 16));
		SetCpuRegister(cpuReg0, static_cast<uint16_t>(value->ival));
	}

	return true;
}

static bool do_step(thid_t tid, resume_mode_t resmod)
{
	switch (resmod)
	{
	case resume_mode_t::RESMOD_INTO:
		{
			Step(CpuType::Cpu, 1, StepType::Step);
		}
		break;
	case resume_mode_t::RESMOD_OVER:
		{
			Step(CpuType::Cpu, 1, StepType::StepOver);
		}
		break;
	case resume_mode_t::RESMOD_OUT:
		{
			Step(CpuType::Cpu, 1, StepType::StepOut);
		}
		break;
	default:
		return false;
	}

	return true;
}

static int idaapi run_mesen_gui(void* ud)
{
	Start((const char*)ud); // will return after Mesen will be closed

	return 0;
}

static bool start_emulation(const char* rom)
{
	mesen_th = qthread_create(run_mesen_gui, (void*)rom);

	while (!IsMesenStarted())
	{
		qsleep(10);
	}

	bool result = register_events_callback();

	emulation_stopped = false;

	return result;
}

static bool exit_emulation(qstring* errbuf)
{
	Stop();

	while (!emulation_stopped)
	{
		qsleep(10);
	}

	bool result = unregister_events_callback();

	CloseGui();

	terminate_gui();

	return result;
}

static bool thread_continue(thid_t tid)
{
	ResumeExecution();
	return true;
}

static bool thread_suspend(thid_t tid)
{
	Pause(ConsoleId::Main);
	return true;
}

static bool get_memory_info(meminfo_vec_t& areas, qstring* errbuf)
{
	//memory_info_t info;

	//info.start_ea = 0x000000;
	//info.end_ea = 0xFFFFFF;
	//info.name.sprnt("CPU");
	//info.sclass.sprnt("DATA");
	//info.perm = 0;
	//info.sbase = 0;
	//info.bitness = 1;

	//areas.push_back(info);

	//auto count = 0;
	//GetMemoryRegions(nullptr, count);

	//if (!count)
	//{
	//	return false;
	//}

	//if (regions == nullptr) {
	//	regions = new mem_region_t[count];
	//	memset(regions, 0, sizeof(mem_region_t) * count);
	//	GetMemoryRegions(regions, count);
	//}

	//for (auto i = 0; i < count; ++i)
	//{
	//	memory_info_t info;
	//	info.start_ea = (regions[i].startBank << 16) | (regions[i].startPage);
	//	info.end_ea = (regions[i].endBank << 16) | (regions[i].endPage);
	//	info.name = regions[i].name;

	//	if (info.name == "PrgROM")
	//	{
	//		info.sclass = "CODE";
	//		info.perm = SEGPERM_READ | SEGPERM_WRITE;
	//	}
	//	else if (info.name.find("Regs") != qstring::npos)
	//	{
	//		info.sclass = "XTRN";
	//		info.perm = SEGPERM_WRITE;
	//	}
	//	else
	//	{
	//		info.sclass = "DATA";
	//		info.perm = SEGPERM_MAXVAL;
	//	}

	//	info.sbase = 0;
	//	info.bitness = 0;

	//	areas.push_back(info);
	//}

	memory_info_t info;

	// Don't remove this loop
	for (int i = 0; i < get_segm_qty(); ++i)
	{
		segment_t* segm = getnseg(i);

		info.start_ea = segm->start_ea;
		info.end_ea = segm->end_ea;

		qstring buf;
		get_segm_name(&buf, segm);
		info.name = buf;

		get_segm_class(&buf, segm);
		info.sclass = buf;

		info.sbase = get_segm_base(segm);
		info.perm = segm->perm;
		info.bitness = segm->bitness;
		areas.push_back(info);
	}
	// Don't remove this loop

	return true;
}

static gdecode_t get_dbg_event(debug_event_t* event, int timeout_ms)
{
	while (true)
	{
		// are there any pending events?
		if (!debug_events.empty())
		{
			*event = debug_events.front();
			debug_events.pop_front();

			return debug_events.empty() ? GDE_ONE_EVENT : GDE_MANY_EVENTS;
		}

		if (debug_events.empty())
		{
			break;
		}
	}

	return GDE_NO_EVENT;
}

static ssize_t idaapi idd_notify(void* user_data, int notification_code, va_list va)
{
	drc_t retcode = DRC_NONE;

	switch (notification_code)
	{
	case debugger_t::ev_init_debugger:
		{
			retcode = init_debugger() ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_term_debugger:
		{
			retcode = release_debugger() ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_start_process:
		{
			const char* path = va_arg(va, const char*);
			const char* args = va_arg(va, const char*);
			const char* startdir = va_arg(va, const char*);
			uint32 dbg_proc_flags = va_arg(va, uint32);
			const char* input_path = va_arg(va, const char*);
			uint32 input_file_crc32 = va_arg(va, uint32);
			qstring* errbuf = va_arg(va, qstring*);

			retcode = start_emulation(input_path) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_get_debapp_attrs:
		{
			debapp_attrs_t* out_pattrs = va_arg(va, debapp_attrs_t*);
			out_pattrs->addrsize = DEF_ADDRSIZE;
			out_pattrs->is_be = false;
			out_pattrs->platform = "snes";
			out_pattrs->cbsize = sizeof(debapp_attrs_t);
			retcode = DRC_OK;
		}
		break;
	case debugger_t::ev_request_pause:
		{
			Pause(ConsoleId::Main);
			retcode = DRC_OK;
		}
		break;
	case debugger_t::ev_exit_process:
		{
			qstring* errbuf = va_arg(va, qstring*);
			retcode = exit_emulation(errbuf) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_get_debug_event:
		{
			gdecode_t* code = va_arg(va, gdecode_t*);
			debug_event_t* event = va_arg(va, debug_event_t*);
			int timeout_ms = va_arg(va, int);
			*code = get_dbg_event(event, timeout_ms);
		}
		break;
	case debugger_t::ev_resume:
		{
			debug_event_t* event = va_arg(va, debug_event_t*);

			switch (event->eid())
			{
			case event_id_t::BREAKPOINT:
			case event_id_t::PROCESS_SUSPENDED:
			case event_id_t::STEP:
				{
					thread_continue(event->tid);
				}
			default:
				retcode = DRC_OK;
			}
		}
		break;
	case debugger_t::ev_thread_suspend:
		{
			thid_t tid = va_arg(va, thid_t);
			retcode = thread_suspend(tid) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_thread_continue:
		{
			thid_t tid = va_arg(va, thid_t);
			retcode = thread_continue(tid) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_set_resume_mode:
		{
			thid_t tid = va_argi(va, thid_t);
			resume_mode_t resmod = va_argi(va, resume_mode_t);
			retcode = do_step(tid, resmod) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_read_registers:
		{
			thid_t tid = va_arg(va, thid_t);
			int clsmask = va_arg(va, int);
			regval_t* values = va_arg(va, regval_t*);
			qstring* errbuf = va_arg(va, qstring*);
			retcode = read_registers(tid, clsmask, values, errbuf) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_write_register:
		{
			thid_t tid = va_argi(va, thid_t);
			int regidx = va_arg(va, int);
			const regval_t* value = va_arg(va, const regval_t*);
			qstring* errbuf = va_arg(va, qstring*);
			retcode = write_register(tid, regidx, value, errbuf) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_get_memory_info:
		{
			meminfo_vec_t* ranges = va_arg(va, meminfo_vec_t*);
			qstring* errbuf = va_arg(va, qstring*);
			retcode = get_memory_info(*ranges, errbuf) ? DRC_OK : DRC_FAILED;
		}
		break;
	case debugger_t::ev_read_memory:
		{
			size_t* nbytes = va_arg(va, size_t*);
			ea_t ea = va_arg(va, ea_t);
			void* buffer = va_arg(va, void*);
			size_t size = va_arg(va, size_t);
			qstring* errbuf = va_arg(va, qstring*);
			retcode = read_memory(ea, buffer, size, errbuf) ? DRC_OK : DRC_FAILED;
			*nbytes = retcode == DRC_OK ? size : 0;
		}
		break;
	case debugger_t::ev_write_memory:
		{
			retcode = DRC_OK;
		}
		break;
	case debugger_t::ev_update_bpts:
		{
			int* nbpts = va_arg(va, int*);
			update_bpt_info_t* bpts = va_arg(va, update_bpt_info_t*);
			int nadd = va_arg(va, int);
			int ndel = va_arg(va, int);
			qstring* errbuf = va_arg(va, qstring*);
			retcode = update_bpts(nbpts, bpts, nadd, ndel, errbuf) ? DRC_OK : DRC_FAILED;
		}
		break;
		//default: {
		// qstring a;
		// a.sprnt("%d\n", notification_code);
		// OutputDebugStringA(a.c_str());
		//}
	}

	return retcode;
}

debugger_t debugger{
	IDD_INTERFACE_VERSION,
	NAME,
	0x8000 + 6581, // (6)
	"65816",

	DBG_FLAG_NOHOST | DBG_FLAG_CAN_CONT_BPT | DBG_FLAG_SAFE | DBG_FLAG_FAKE_ATTACH | DBG_FLAG_NOPASSWORD |
	DBG_FLAG_NOPARAMETERS | DBG_FLAG_ANYSIZE_HWBPT | DBG_FLAG_DEBTHREAD,
	DBG_HAS_REQUEST_PAUSE | DBG_HAS_SET_RESUME_MODE | DBG_HAS_THREAD_SUSPEND | DBG_HAS_THREAD_CONTINUE
	/*| DBG_HAS_MAP_ADDRESS*/,

	register_classes,
	RC_CPU,
	registers,
	qnumber(registers),

	0x1000,

	NULL,
	0,
	0,

	DBG_RESMOD_STEP_INTO | DBG_RESMOD_STEP_OVER | DBG_RESMOD_STEP_OUT,

	NULL,
	idd_notify
};
