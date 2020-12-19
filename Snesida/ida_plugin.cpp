#include <ida.hpp>
#include <idp.hpp>
#include <dbg.hpp>
#include <loader.hpp>

#include "ida_plugin.h"

extern debugger_t debugger;

static bool plugin_inited;

static bool init_plugin(void)
{
	return (ph.id == PLFM_65C816);
}

static void print_version()
{
	static const char format[] = NAME " debugger plugin v%s;\nAuthor: Dr. MefistO [Lab 313] <doc.mefisto@gmail.com>.";
	info(format, VERSION);
	msg(format, VERSION);
}

static plugmod_t* idaapi init(void)
{
	if (init_plugin())
	{
		dbg = &debugger;
		plugin_inited = true;

		print_version();
		return PLUGIN_KEEP;
	}

	return PLUGIN_SKIP;
}

static void idaapi term(void)
{
	if (plugin_inited)
	{
		plugin_inited = false;
	}
}

static bool idaapi run(size_t arg)
{
	return false;
}

char comment[] = NAME " debugger plugin by Dr. MefistO.";

char help[] =
	NAME " debugger plugin by Dr. MefistO.\n"
	"\n"
	"This module lets you debug SNES roms in IDA.\n";

plugin_t PLUGIN = {
	IDP_INTERFACE_VERSION,
	PLUGIN_PROC | PLUGIN_DBG | PLUGIN_MOD,
	init,
	term,
	run,
	comment,
	help,
	NAME " debugger plugin",
	""
};
