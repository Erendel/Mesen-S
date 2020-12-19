#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/IAudioDevice.h"
#include "../Core/EmuSettings.h"
#include "../Core/SettingTypes.h"

extern shared_ptr<Console> _console;
extern unique_ptr<IAudioDevice> _soundManager;
static string _returnString;

extern "C" {
DllExport void __stdcall SetVideoConfig(VideoConfig config)
{
	if (_console.get())
	{
		_console->GetSettings()->SetVideoConfig(config);
	}
}

DllExport void __stdcall SetAudioConfig(AudioConfig config)
{
	if (_console.get())
	{
		_console->GetSettings()->SetAudioConfig(config);
	}
}

DllExport void __stdcall SetInputConfig(InputConfig config)
{
	if (_console.get())
	{
		_console->GetSettings()->SetInputConfig(config);
	}
}

DllExport void __stdcall SetEmulationConfig(EmulationConfig config)
{
	if (_console.get())
	{
		_console->GetSettings()->SetEmulationConfig(config);
	}
}

DllExport void __stdcall SetGameboyConfig(GameboyConfig config)
{
	if (_console.get())
	{
		_console->GetSettings()->SetGameboyConfig(config);
	}
}

DllExport void __stdcall SetPreferences(PreferencesConfig config)
{
	if (_console.get())
	{
		_console->GetSettings()->SetPreferences(config);
	}
}

DllExport void __stdcall SetShortcutKeys(ShortcutKeyInfo shortcuts[], uint32_t count)
{
	vector<ShortcutKeyInfo> shortcutList(shortcuts, shortcuts + count);

	if (_console.get())
	{
		_console->GetSettings()->SetShortcutKeys(shortcutList);
	}
}

DllExport ControllerType __stdcall GetControllerType(int player)
{
	return _console.get() ? _console->GetSettings()->GetInputConfig().Controllers[player].Type : ControllerType::None;
}

DllExport const char* __stdcall GetAudioDevices()
{
	_returnString = _soundManager.get() ? _soundManager->GetAvailableDevices() : "";
	return _returnString.c_str();
}

DllExport void __stdcall SetEmulationFlag(EmulationFlags flag, bool enabled)
{
	if (_console.get())
	{
		_console->GetSettings()->SetFlagState(flag, enabled);
	}
}

DllExport void __stdcall SetDebuggerFlag(DebuggerFlags flag, bool enabled)
{
	if (_console.get())
	{
		_console->GetSettings()->SetDebuggerFlag(flag, enabled);
	}
}
}
