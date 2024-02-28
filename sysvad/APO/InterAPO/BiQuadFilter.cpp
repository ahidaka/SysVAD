#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <mmreg.h>

#include <audioenginebaseapo.h>
#include <baseaudioprocessingobject.h>
#include <resource.h>

#include <float.h>

#include "InterAPO.h"
#include "BiQuadFilter.h"

#include <EvnTrace.h>
#include "trace.h"
#include "BiQuadFilter.tmh"

//
// Implements BiQuad filtering based on the methods from the Robert Bristow-Johnson Audio EQ Cookbook.
// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
//

void BiQuadFilter::Prepare(float SampleRate, float Q, float Gain, float Frequency, int FilterType)
{
	SetSampleRate(SampleRate);
	SetQ(Q);
	SetGain(Gain);
	SetFrequency(Frequency);
	nFilterType= (enum FilterType)FilterType;
}

void BiQuadFilter::Prepare(LONG Gain)
{
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! Gain=%ld", Gain);

	SetGain((float)Gain);
	switch (nFilterType)
	{
	case LowPass:
		SetLowPass();
		break;

	case HighPass:
		SetHighPass();
		break;

	case BandPass:
		SetBandPass();
		break;

	case Notch:
		SetNotch();
		break;

	case Peak:
		SetPeak();
		break;

	case LowShelf:
		SetLowShelf();
		break;

	case HighShelf:
		SetHighShelf();
		break;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
		"<-- %!FUNC! %d:%f %f,%f,%f,%f", Gain, fGainLevel, b0, b2, a0, a2);
}

float BiQuadFilter::Process(float Input, float SampleRate, float Q, float Gain, float Frequency, int FilterType)
{
	float Output = 0.0f;

	Prepare(SampleRate, Q, Gain, Frequency, FilterType);

	Output = b0 / a0 * Input + b1 / a0 * x1 + b2 / a0 * x2 - a1 / a0 * y1 - a2 / a0 * y2;

	x2 = x1;
	x1 = Input;
	y2 = y1;
	y1 = Output;

	return Output;
}

float BiQuadFilter::Process(float Input)
{
	float Output = 0.0f;

	Output = b0 / a0 * Input + b1 / a0 * x1 + b2 / a0 * x2 - a1 / a0 * y1 - a2 / a0 * y2;

	x2 = x1;
	x1 = Input;
	y2 = y1;
	y1 = Output;

	return Output;
}
