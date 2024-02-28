//
// BiQuadFilter.h -- 
//
// Description:
//
//   Declaration of the BiQuadFilter class.
//

#pragma once

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

//
// Implements BiQuad filtering based on the methods from the Robert Bristow-Johnson Audio EQ Cookbook.
// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
//

class  BiQuadFilter
{
public:

	enum FilterType
	{
		LowPass,
		HighPass,
		BandPass,
		Notch,
		Peak,
		LowShelf,
		HighShelf
	};

	BiQuadFilter()
		:
		a0(0.0f),
		a1(0.0f),
		a2(0.0f),
		b0(0.0f),
		b1(0.0f),
		b2(0.0f),
		x1(0.0f),
		x2(0.0f),
		y1(0.0f),
		y2(0.0f),
		fSampleRate(0.0f),
		fQ(1.0f),
		fGain(0.0f),
		fFrequency(0.0f),
		fGainLevel(0.0f),
		f0(0.0f),
		nFilterType(Peak)
	{}
	~BiQuadFilter() {}

	void SetSampleRate(float SampleRate)
	{
		this->fSampleRate = SampleRate;
	}

	void SetFrequency(float Frequency)
	{
		this->fFrequency = Frequency;
	}

	void SetQ(float Q)
	{
		this->fQ = Q;
	}

	void SetGain(float Gain)
	{
		// Gain: -20.0 .. 0 .. +20.0
		this->fGain = Gain;
		this->fGainLevel = powf(10.0f, Gain / 20.0f);
	}

	void SetGainlevel(float GainLevel)
	{
		// GainLevel: 0.1 ... 10.0
		this->fGainLevel = GainLevel;
	}

	void SetLowPass()
	{
		float omega = 2.0f * PI * fFrequency / fSampleRate;
		float sn = sinf(omega);
		float cs = cosf(omega);
		float alpha = sn / (2.0f * fQ);

		b0 = (1.0f - cs) / 2.0f;
		b1 = 1.0f - cs;
		b2 = (1.0f - cs) / 2.0f;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;
	}

	void SetHighPass()
	{
		float omega = 2.0f * PI * fFrequency / fSampleRate;
		float sn = sinf(omega);
		float cs = cosf(omega);
		float alpha = sn / (2.0f * fQ);

		b0 = (1.0f + cs) / 2.0f;
		b1 = -(1.0f + cs);
		b2 = (1.0f + cs) / 2.0f;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;
	}

	void SetBandPass()
	{
		float omega = 2.0f * PI * fFrequency / fSampleRate;
		float sn = sinf(omega);
		float cs = cosf(omega);
		float alpha = sn / (2.0f * fQ);

		b0 = sn / 2.0f;
		b1 = 0.0f;
		b2 = -sn / 2.0f;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;
	}

	void SetNotch()
	{
		f0 = fFrequency / fSampleRate;
		float omega = 2.0f * PI * f0;
		float sn = sinf(omega);
		float cs = cosf(omega);
		float alpha = sn / (2.0f * fQ);

		b0 = 1.0f;
		b1 = -2.0f * cs;
		b2 = 1.0f;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;
	}

	void SetPeak()
	{
		f0 = fFrequency / fSampleRate;
		float omega = 2.0f * PI * f0;
		float sn = sinf(omega);
		float cs = cosf(omega);
		float alpha = sn / (2.0f * fQ);

		b0 = 1.0f + alpha * fGainLevel;
		b1 = -2.0f * cs;
		b2 = 1.0f - alpha * fGainLevel;
		a0 = 1.0f + alpha / fGainLevel;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha / fGainLevel;
	}

	void SetLowShelf()
	{
		float value = powf(10.0f, fGainLevel / 40.0f);
		f0 = fFrequency / fSampleRate;  // cutoff frequency
		float omega = 2.0f * PI * f0;
		float sn = sinf(omega);
		float cs = cosf(omega);
		//float alpha = sn / (2.0f * fQ);
		float beta = sqrtf(value) / fQ;

		b0 = value * ((value + 1.0f) - (value - 1.0f) * cs + beta * sn);
		b1 = 2.0f * value * ((value - 1.0f) - (value + 1.0f) * cs);
		b2 = value * ((value + 1.0f) - (value - 1.0f) * cs - beta * sn);
		a0 = (value + 1.0f) + (value - 1.0f) * cs + beta * sn;
		a1 = -2.0f * ((value - 1.0f) + (value + 1.0f) * cs);
		a2 = (value + 1.0f) + (value - 1.0f) * cs - beta * sn;
	}

	void SetHighShelf()
	{
		float value = powf(10.0f, fGainLevel / 40.0f);
		f0 = fFrequency / fSampleRate;  // cutoff frequency
		float omega = 2.0f * PI * f0;
		float sn = sinf(omega);
		float cs = cosf(omega);
		//float alpha = sn / (2.0f * fQ);
		float beta = sqrtf(value) / fQ;

		b0 = value * ((value + 1.0f) + (value - 1.0f) * cs + beta * sn);
		b1 = -2.0f * value * ((value - 1.0f) + (value + 1.0f) * cs);
		b2 = value * ((value + 1.0f) + (value - 1.0f) * cs - beta * sn);
		a0 = (value + 1.0f) - (value - 1.0f) * cs + beta * sn;
		a1 = 2.0f * ((value - 1.0f) - (value + 1.0f) * cs);
		a2 = (value + 1.0f) - (value - 1.0f) * cs - beta * sn;
	}

	void SetFilterType(int FilterType)
	{
		switch (FilterType)
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
		}
	}
private:
	const float PI = 3.14159265358979323846f;

	float a0, a1, a2, b0, b1, b2; // filter coefficients
	float x1, x2, y1, y2; // filter delay elements
	float fSampleRate;
	float fQ;
	float fGain;
	float fFrequency;
	float fGainLevel;
	float f0;
	enum FilterType nFilterType;

public:
	void Prepare(float fSampleRate, float fQ, float fGain, float fFrequency, int nFilterType);
	void Prepare(LONG Gain);
	float Process(float fInput, float fSampleRate, float fQ, float fGain, float fFrequency, int nFilterType);
	float Process(float fInput);
};

//
//   Declaration of the GraphicEqualizer routine.
//
class GraphicEqualizer
{
public:
	// constructor
	GraphicEqualizer()
	{
		// Initialize the filters, 3 bands
		// 125 Hz, 500 Hz, 4000 Hz
		//                  SampleRate, Q, Gain, Frequency, FilterType
		m_LowFilter.Prepare(48000.0f, 1.0f, 0.0f, 125.0f, BiQuadFilter::Peak);
		m_MidFilter.Prepare(48000.0f, 1.0f, 0.0f, 500.0f, BiQuadFilter::Peak);
		m_HighFilter.Prepare(48000.0f, 1.0f, 0.0f, 4000.0f, BiQuadFilter::Peak);
	}

	// destructor
	~GraphicEqualizer() {}

public:
	BiQuadFilter m_LowFilter;
	BiQuadFilter m_MidFilter;
	BiQuadFilter m_HighFilter;
};
