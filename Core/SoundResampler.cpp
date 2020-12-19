#include "stdafx.h"
#include "SoundResampler.h"
#include "Console.h"
#include "Spc.h"
#include "EmuSettings.h"
#include "SoundMixer.h"
#include "VideoRenderer.h"
#include "../Utilities/HermiteResampler.h"

SoundResampler::SoundResampler(Console* console)
{
	_console = console;
}

SoundResampler::~SoundResampler()
{
}

double SoundResampler::GetRateAdjustment()
{
	return _rateAdjustment;
}

double SoundResampler::GetTargetRateAdjustment()
{
	AudioConfig cfg = _console->GetSettings()->GetAudioConfig();
	bool isRecording = _console->GetSoundMixer()->IsRecording() || _console->GetVideoRenderer()->IsRecording();
	if (!isRecording && !cfg.DisableDynamicSampleRate)
	{
		//Don't deviate from selected sample rate while recording
		//TODO: Have 2 output streams (one for recording, one for the speakers)
		AudioStatistics stats = _console->GetSoundMixer()->GetStatistics();

		if (stats.AverageLatency > 0 && _console->GetSettings()->GetEmulationSpeed() == 100)
		{
			//Try to stay within +/- 3ms of requested latency
			constexpr int32_t maxGap = 3;
			constexpr int32_t maxSubAdjustment = 3600;

			int32_t requestedLatency = (int32_t)cfg.AudioLatency;
			double latencyGap = stats.AverageLatency - requestedLatency;
			double adjustment = std::min(0.0025, (std::ceil((std::abs(latencyGap) - maxGap) * 8)) * 0.00003125);

			if (latencyGap < 0 && _underTarget < maxSubAdjustment)
			{
				_underTarget++;
			}
			else if (latencyGap > 0 && _underTarget > -maxSubAdjustment)
			{
				_underTarget--;
			}

			//For every ~1 second spent under/over target latency, further adjust rate (GetTargetRate is called approx. 3x per frame) 
			//This should slowly get us closer to the actual output rate of the sound card
			double subAdjustment = 0.00003125 * _underTarget / 180;

			if (adjustment > 0)
			{
				if (latencyGap > maxGap)
				{
					_rateAdjustment = 1 - adjustment + subAdjustment;
				}
				else if (latencyGap < -maxGap)
				{
					_rateAdjustment = 1 + adjustment + subAdjustment;
				}
			}
			else if (std::abs(latencyGap) < 1)
			{
				//Restore normal rate once we get within +/- 1ms
				_rateAdjustment = 1.0 + subAdjustment;
			}
		}
		else
		{
			_underTarget = 0;
			_rateAdjustment = 1.0;
		}
	}
	else
	{
		_underTarget = 0;
		_rateAdjustment = 1.0;
	}
	return _rateAdjustment;
}

void SoundResampler::UpdateTargetSampleRate(uint32_t sourceRate, uint32_t sampleRate)
{
	double spcSampleRate = sourceRate;
	if (_console->GetSettings()->GetVideoConfig().IntegerFpsMode)
	{
		//Adjust sample rate when running at 60.0 fps instead of 60.1
		switch (_console->GetRegion())
		{
		default:
		case ConsoleRegion::Ntsc: spcSampleRate = sourceRate * (60.0 / _console->GetFps());
			break;
		case ConsoleRegion::Pal: spcSampleRate = sourceRate * (50.0 / _console->GetFps());
			break;
		}
	}

	double targetRate = sampleRate * GetTargetRateAdjustment();
	if (targetRate != _previousTargetRate || spcSampleRate != _prevSpcSampleRate)
	{
		_previousTargetRate = targetRate;
		_prevSpcSampleRate = spcSampleRate;
		_resampler.SetSampleRates(spcSampleRate, targetRate);
	}
}

uint32_t SoundResampler::Resample(int16_t* inSamples, uint32_t sampleCount, uint32_t sourceRate, uint32_t sampleRate,
                                  int16_t* outSamples)
{
	UpdateTargetSampleRate(sourceRate, sampleRate);
	return _resampler.Resample(inSamples, sampleCount, outSamples);
}
