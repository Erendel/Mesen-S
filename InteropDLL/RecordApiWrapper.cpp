#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/VideoRenderer.h"
#include "../Core/SoundMixer.h"
#include "../Core/MovieManager.h"

extern shared_ptr<Console> _console;
enum class VideoCodec;

extern "C" {
DllExport void __stdcall AviRecord(char* filename, VideoCodec codec, uint32_t compressionLevel)
{
	if (_console)
	{
		_console->GetVideoRenderer()->StartRecording(filename, codec, compressionLevel);
	}
}

DllExport void __stdcall AviStop()
{
	if (_console)
	{
		_console->GetVideoRenderer()->StopRecording();
	}
}

DllExport bool __stdcall AviIsRecording()
{
	return _console ? _console->GetVideoRenderer()->IsRecording() : false;
}

DllExport void __stdcall WaveRecord(char* filename)
{
	if (_console)
	{
		_console->GetSoundMixer()->StartRecording(filename);
	}
}

DllExport void __stdcall WaveStop()
{
	if (_console)
	{
		_console->GetSoundMixer()->StopRecording();
	}
}

DllExport bool __stdcall WaveIsRecording()
{
	return _console ? _console->GetSoundMixer()->IsRecording() : false;
}

DllExport void __stdcall MoviePlay(char* filename)
{
	if (_console)
	{
		_console->GetMovieManager()->Play(string(filename));
	}
}

DllExport void __stdcall MovieStop()
{
	if (_console)
	{
		_console->GetMovieManager()->Stop();
	}
}

DllExport bool __stdcall MoviePlaying()
{
	return _console ? _console->GetMovieManager()->Playing() : false;
}

DllExport bool __stdcall MovieRecording()
{
	return _console ? _console->GetMovieManager()->Recording() : false;
}

DllExport void __stdcall MovieRecord(RecordMovieOptions* options)
{
	RecordMovieOptions opt = *options;

	if (_console)
	{
		_console->GetMovieManager()->Record(opt);
	}
}
}
