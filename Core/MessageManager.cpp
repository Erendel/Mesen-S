#include "stdafx.h"
#include "MessageManager.h"
#include "EmuSettings.h"

std::unordered_map<string, string> MessageManager::_enResources = {
	{"Cheats", u8"Cheats"},
	{"Debug", u8"Debug"},
	{"EmulationSpeed", u8"Emulation Speed"},
	{"ClockRate", u8"Clock Rate"},
	{"Error", u8"Error"},
	{"GameInfo", u8"Game Info"},
	{"GameLoaded", u8"Game loaded"},
	{"Input", u8"Input"},
	{"Patch", u8"Patch"},
	{"Movies", u8"Movies"},
	{"NetPlay", u8"Net Play"},
	{"Overclock", u8"Overclock"},
	{"Region", u8"Region"},
	{"SaveStates", u8"Save States"},
	{"ScreenshotSaved", u8"Screenshot Saved"},
	{"SoundRecorder", u8"Sound Recorder"},
	{"Test", u8"Test"},
	{"VideoRecorder", u8"Video Recorder"},

	{"ApplyingPatch", u8"Applying patch: %1"},
	{"CheatApplied", u8"1 cheat applied."},
	{"CheatsApplied", u8"%1 cheats applied."},
	{"CheatsDisabled", u8"All cheats disabled."},
	{"CoinInsertedSlot", u8"Coin inserted (slot %1)"},
	{"ConnectedToServer", u8"Connected to server."},
	{"ConnectedAsPlayer", u8"Connected as player %1"},
	{"ConnectedAsSpectator", u8"Connected as spectator."},
	{"ConnectionLost", u8"Connection to server lost."},
	{"CouldNotConnect", u8"Could not connect to the server."},
	{"CouldNotInitializeAudioSystem", u8"Could not initialize audio system"},
	{"CouldNotFindRom", u8"Could not find matching game ROM. (%1)"},
	{"CouldNotWriteToFile", u8"Could not write to file: %1"},
	{"CouldNotLoadFile", u8"Could not load file: %1"},
	{"EmulationMaximumSpeed", u8"Maximum speed"},
	{"EmulationSpeedPercent", u8"%1%"},
	{"FdsDiskInserted", u8"Disk %1 Side %2 inserted."},
	{"Frame", u8"Frame"},
	{"GameCrash", u8"Game has crashed (%1)"},
	{"KeyboardModeDisabled", u8"Keyboard mode disabled."},
	{"KeyboardModeEnabled", u8"Keyboard mode enabled."},
	{"Lag", u8"Lag"},
	{"Mapper", u8"Mapper: %1, SubMapper: %2"},
	{"MovieEnded", u8"Movie ended."},
	{"MovieInvalid", u8"Invalid movie file."},
	{"MovieMissingRom", u8"Missing ROM required (%1) to play movie."},
	{
		"MovieNewerVersion",
		u8"Cannot load movies created by a more recent version of Mesen-S. Please download the latest version."
	},
	{"MovieIncompatibleVersion", u8"This movie is incompatible with this version of Mesen-S."},
	{"MoviePlaying", u8"Playing movie: %1"},
	{"MovieRecordingTo", u8"Recording to: %1"},
	{"MovieSaved", u8"Movie saved to file: %1"},
	{"NetplayVersionMismatch", u8"%1 is not running the same version of Mesen-S and has been disconnected."},
	{"NetplayNotAllowed", u8"This action is not allowed while connected to a server."},
	{"OverclockEnabled", u8"Overclocking enabled."},
	{"OverclockDisabled", u8"Overclocking disabled."},
	{"PrgSizeWarning", u8"PRG size is smaller than 32kb"},
	{"SaveStateEmpty", u8"Slot is empty."},
	{"SaveStateIncompatibleVersion", u8"Save state is incompatible with this version of Mesen-S."},
	{"SaveStateInvalidFile", u8"Invalid save state file."},
	{"SaveStateWrongSystemSnes", u8"Error: State cannot be loaded (wrong console type: SNES)"},
	{"SaveStateWrongSystemGb", u8"Error: State cannot be loaded (wrong console type: Game Boy)"},
	{"SaveStateLoaded", u8"State #%1 loaded."},
	{"SaveStateMissingRom", u8"Missing ROM required (%1) to load save state."},
	{
		"SaveStateNewerVersion",
		u8"Cannot load save states created by a more recent version of Mesen-S. Please download the latest version."
	},
	{"SaveStateSaved", u8"State #%1 saved."},
	{"SaveStateSlotSelected", u8"Slot #%1 selected."},
	{"ScanlineTimingWarning", u8"PPU timing has been changed."},
	{"ServerStarted", u8"Server started (Port: %1)"},
	{"ServerStopped", u8"Server stopped"},
	{"SoundRecorderStarted", u8"Recording to: %1"},
	{"SoundRecorderStopped", u8"Recording saved to: %1"},
	{"TestFileSavedTo", u8"Test file saved to: %1"},
	{"UnsupportedMapper", u8"Unsupported mapper (%1), cannot load game."},
	{"VideoRecorderStarted", u8"Recording to: %1"},
	{"VideoRecorderStopped", u8"Recording saved to: %1"},
};

std::unordered_map<string, string> MessageManager::_deResources = {
	{"Cheats", u8"Cheats"},
	{"Debug", u8"Debug"},
	{"EmulationSpeed", u8"Emulationsgeschwindigkeit"},
	{"ClockRate", u8"Taktrate"},
	{"Error", u8"Fehler"},
	{"GameInfo", u8"Spielinfo"},
	{"GameLoaded", u8"Spiel geladen"},
	{"Input", u8"Eingabe"},
	{"Patch", u8"Patch"},
	{"Movies", u8"Filme"},
	{"NetPlay", u8"Net Play"},
	{"Overclock", u8"Übertaktung"},
	{"Region", u8"Region"},
	{"SaveStates", u8"Zwischenstände"},
	{"ScreenshotSaved", u8"Screenshot gespeicher"},
	{"SoundRecorder", u8"Tonaufnahme"},
	{"Test", u8"Test"},
	{"VideoRecorder", u8"Videoaufnahme"},

	{"ApplyingPatch", u8"Wende Patch an: %1"},
	{"CheatApplied", u8"1 Cheat angewendet."},
	{"CheatsApplied", u8"%1 Cheats angewendet."},
	{"CheatsDisabled", u8"Alle Cheats deaktiviert."},
	{"CoinInsertedSlot", u8"Münze eingeworfen (Schlitz %1)"},
	{"ConnectedToServer", u8"Mit dem Server verbunden."},
	{"ConnectedAsPlayer", u8"Verbunden als Spieler %1"},
	{"ConnectedAsSpectator", u8"Verbunden als Beobachter."},
	{"ConnectionLost", u8"Verbindung zum Server unterbrochen."},
	{"CouldNotConnect", u8"Konnte keine Verbindung zum Server aufbauen."},
	{"CouldNotInitializeAudioSystem", u8"Konnte Tonsystem nicht initialisieren"},
	{"CouldNotFindRom", u8"Konnte passende Spiel - ROM nicht finden. (% 1)"},
	{"CouldNotWriteToFile", u8"Konnte nicht in Datei schreiben: %1"},
	{"CouldNotLoadFile", u8"Datei konnte nicht geladen werden: %1"},
	{"EmulationMaximumSpeed", u8"Maximalgeschwindigkeit"},
	{"EmulationSpeedPercent", u8"%1%"},
	{"FdsDiskInserted", u8"Diskette %1 Seite %2 eingelegt."},
	{"Frame", u8"Bild"},
	{"GameCrash", u8"Das Spiel ist abgestürzt (%1)"},
	{"KeyboardModeDisabled", u8"Tastaturmodus deaktiviert."},
	{"KeyboardModeEnabled", u8"Tastaturmodus aktiviert."},
	{"Lag", u8"Verzögerung"},
	{"Mapper", u8"Mapper: %1, SubMapper: %2"},
	{"MovieEnded", u8"Film beendet."},
	{"MovieInvalid", u8"Ungültige Filmdatei."},
	{"MovieMissingRom", u8"Fehlendes ROM (%1) benötigt, um Film wiederzugeben."},
	{"MovieNewerVersion", u8"Filme, die mit einer neueren Version von Mesen-S erstellt wurden, können nicht geladen  werden. Bitte laden Sie die aktuellste Version herunter."},
	{"MovieIncompatibleVersion", u8"Dieser Film ist mit dieser Version von Mesen-S inkompatibel."},
	{"MoviePlaying", u8"Gebe Film wieder: %1"},
	{"MovieRecordingTo", u8"Nehme Film in Datei auf: %1"},
	{"MovieSaved", u8"Film wurde in Datei gespeichert: %1"},
	{"NetplayVersionMismatch", u8"%1 führt nicht dieselbe Version von Mesen-S aus und wurde getrennt."},
	{"NetplayNotAllowed", u8"Diese Aktion ist verboten, während Sie mit einem Server verbunden sind."},
	{"OverclockEnabled", u8"Übertaktung aktiviert."},
	{"OverclockDisabled", u8"Übertaktung deaktiviert."},
	{"PrgSizeWarning", u8"PRG-Größe ist kleiner als 32kb"},
	{"SaveStateEmpty", u8"Zwischenstand-Platz ist leer."},
	{"SaveStateIncompatibleVersion", u8"Zwischenstand ist inkompatibel mit dieser Version von Mesen-S."},
	{"SaveStateInvalidFile", u8"Ungültige Zwischenstanddatei."},
	{"SaveStateWrongSystemSnes", u8"Fehler: Zwischenstand kann nicht geladen werden (falscher Konsolentyp: SNES)"},
	{"SaveStateWrongSystemGb", u8"Fehler: Zwischenstand kann nicht geladen werden (falscher Konsolentyp: Game Boy)"},
	{"SaveStateLoaded", u8"Zwischenstand #%1 geladen."},
	{"SaveStateMissingRom", u8"Fehlende ROM (%1) benötigt, um Zwischenstand zu laden."},
	{"SaveStateNewerVersion", u8"Zwischenstände, die mit einer neueren Version von Mesen-S erstellt wurden, können nicht geladen werden. Bitte laden Sie die aktuellste Version herunter."	},
	{"SaveStateSaved", u8"Zwischenstand #%1 gespeichert."},
	{"SaveStateSlotSelected", u8"Platz #%1 ausgewählt."},
	{"ScanlineTimingWarning", u8"PPU-Timing wurde verändert."},
	{"ServerStarted", u8"Server gestartet (Port: %1)"},
	{"ServerStopped", u8"Server gestoppt"},
	{"SoundRecorderStarted", u8"Nehme Ton in Datei auf: %1"},
	{"SoundRecorderStopped", u8"Tonaufnahme wurde in Datei gespeichert: %1"},
	{"TestFileSavedTo", u8"Testdatei wurde gespeichert in: %1"},
	{"UnsupportedMapper", u8"Nicht unterstützter Mapper (%1), Spiel kann nicht geladen werden."},
	{"VideoRecorderStarted", u8"Nehme Film in Datei auf: %1"},
	{"VideoRecorderStopped", u8"Film wurde gespeichert in: %1"},
};

std::list<string> MessageManager::_log;
SimpleLock MessageManager::_logLock;
SimpleLock MessageManager::_messageLock;
bool MessageManager::_osdEnabled = true;
IMessageManager* MessageManager::_messageManager = nullptr;

void MessageManager::RegisterMessageManager(IMessageManager* messageManager)
{
	auto lock = _messageLock.AcquireSafe();
	MessageManager::_messageManager = messageManager;
}

void MessageManager::UnregisterMessageManager(IMessageManager* messageManager)
{
	auto lock = _messageLock.AcquireSafe();
	if (MessageManager::_messageManager == messageManager)
	{
		MessageManager::_messageManager = nullptr;
	}
}

void MessageManager::SetOsdState(bool enabled)
{
	_osdEnabled = enabled;
}

string MessageManager::Localize(string key)
{
	std::unordered_map<string, string>* resources = &_enResources;
	switch(EmuSettings::GetDisplayLanguage()) {
		case Language::English: resources = &_enResources; break;
		case Language::German: resources = &_deResources; break;
	}
	if (resources)
	{
		if (resources->find(key) != resources->end())
		{
			return (*resources)[key];
		} else if(EmuSettings::GetDisplayLanguage() != Language::English) {
			//Fallback on English if resource key is missing another language
			resources = &_enResources;
			if(resources->find(key) != resources->end()) {
				return (*resources)[key];
			}
		}
	}

	return key;
}

void MessageManager::DisplayMessage(string title, string message, string param1, string param2)
{
	if (MessageManager::_messageManager)
	{
		auto lock = _messageLock.AcquireSafe();
		if (!MessageManager::_messageManager)
		{
			return;
		}

		title = Localize(title);
		message = Localize(message);

		size_t startPos = message.find(u8"%1");
		if (startPos != std::string::npos)
		{
			message.replace(startPos, 2, param1);
		}

		startPos = message.find(u8"%2");
		if (startPos != std::string::npos)
		{
			message.replace(startPos, 2, param2);
		}

		if (_osdEnabled)
		{
			MessageManager::_messageManager->DisplayMessage(title, message);
		}
		else
		{
			MessageManager::Log("[" + title + "] " + message);
		}
	}
}

void MessageManager::Log(string message)
{
#ifndef LIBRETRO
	auto lock = _logLock.AcquireSafe();
	if (message.empty())
	{
		message = "------------------------------------------------------";
	}
	if (_log.size() >= 1000)
	{
		_log.pop_front();
	}
	_log.push_back(message);
#else
	if(MessageManager::_messageManager) {
		MessageManager::_messageManager->DisplayMessage("", message + "\n");
	}
#endif
}

void MessageManager::ClearLog()
{
	auto lock = _logLock.AcquireSafe();
	_log.clear();
}

string MessageManager::GetLog()
{
	auto lock = _logLock.AcquireSafe();
	stringstream ss;
	for (string& msg : _log)
	{
		ss << msg << "\n";
	}
	return ss.str();
}
