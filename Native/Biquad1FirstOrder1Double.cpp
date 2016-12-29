#include "stdafx.h"

#include "Biquad1FirstOrder1Double.h"

namespace CrossTimeDsp::Dsp
{
	#pragma unmanaged
	Biquad1FirstOrder1Double::Biquad1FirstOrder1Double(BiquadCoefficients biquad, FirstOrderCoefficients firstOrder, __int32 channels)
	{
		this->channels = channels;
		this->biquad_a1 = biquad.A1;
		this->biquad_a2 = biquad.A2;
		this->biquad_b0 = biquad.B0;
		this->biquad_b1 = biquad.B1;
		this->biquad_b2 = biquad.B2;

		this->biquad_x1 = new double[channels];
		this->biquad_x2 = new double[channels];
		this->biquad_y1 = new double[channels];
		this->biquad_y2 = new double[channels];

		this->firstOrder_a1 = firstOrder.A1;
		this->firstOrder_b0 = firstOrder.B0;
		this->firstOrder_b1 = firstOrder.B1;

		this->firstOrder_x1 = new double[channels];
		this->firstOrder_y1 = new double[channels];

		for (__int32 channel = 0; channel < channels; ++channel)
		{
			this->biquad_x1[channel] = 0.0;
			this->biquad_x2[channel] = 0.0;
			this->biquad_y1[channel] = 0.0;
			this->biquad_y2[channel] = 0.0;

			this->firstOrder_x1[channel] = 0.0;
			this->firstOrder_y1[channel] = 0.0;
		}
	}

	Biquad1FirstOrder1Double::~Biquad1FirstOrder1Double()
	{
		delete this->biquad_x1;
		delete this->biquad_x2;
		delete this->biquad_y1;
		delete this->biquad_y2;

		delete this->firstOrder_x1;
		delete this->firstOrder_y1;
	}

	void Biquad1FirstOrder1Double::Filter(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			__int32 channel = sample % this->channels;
			double value = block[sample];
			double accumulator = this->biquad_b0 * value + this->biquad_b1 * this->biquad_x1[channel] + this->biquad_b2 * this->biquad_x2[channel] - this->biquad_a1 * this->biquad_y1[channel] - this->biquad_a2 * this->biquad_y2[channel];
			this->biquad_x2[channel] = this->biquad_x1[channel];
			this->biquad_x1[channel] = value;
			this->biquad_y2[channel] = this->biquad_y1[channel];
			this->biquad_y1[channel] = accumulator;

			value = accumulator;
			accumulator = this->firstOrder_b0 * value + this->firstOrder_b1 * this->firstOrder_x1[channel] - this->firstOrder_a1 * this->firstOrder_y1[channel];
			this->firstOrder_x1[channel] = value;
			this->firstOrder_y1[channel] = accumulator;
			block[sample] = accumulator;
		}
	}

	void Biquad1FirstOrder1Double::FilterReverse(double* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInDoubles;
		for (__int32 sample = maxSample - 1; sample >= offset; --sample)
		{
			__int32 channel = sample % this->channels;
			double value = block[sample];
			double accumulator = this->biquad_b0 * value + this->biquad_b1 * this->biquad_x1[channel] + this->biquad_b2 * this->biquad_x2[channel] - this->biquad_a1 * this->biquad_y1[channel] - this->biquad_a2 * this->biquad_y2[channel];
			this->biquad_x2[channel] = this->biquad_x1[channel];
			this->biquad_x1[channel] = value;
			this->biquad_y2[channel] = this->biquad_y1[channel];
			this->biquad_y1[channel] = accumulator;

			value = accumulator;
			accumulator = this->firstOrder_b0 * value + this->firstOrder_b1 * this->firstOrder_x1[channel] - this->firstOrder_a1 * this->firstOrder_y1[channel];
			this->firstOrder_x1[channel] = value;
			this->firstOrder_y1[channel] = accumulator;
			block[sample] = accumulator;
		}
	}
}