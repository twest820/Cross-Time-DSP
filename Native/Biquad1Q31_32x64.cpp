#include "stdafx.h"
#include <cmath>
#include <initializer_list>
#include "Biquad1Q31_32x64.h"
#include "Constant.h"

namespace CrossTimeDsp::Dsp
{
	Biquad1Q31_32x64::Biquad1Q31_32x64(BiquadCoefficients coefficients, __int32 channels)
	{
		// scale coefficients __int32o Q0.31 so that state variables can be stored in Q0.63 for maximum precision
		// the post shift is used to undo the scaling at output
		this->postShift = Q31::GetQ31_32x64_PostShift(std::initializer_list<double>({ coefficients.A1, coefficients.A2, coefficients.B0, coefficients.B1, coefficients.B2 }));
		double coefficientScaling = std::pow(2.0, -this->postShift);
		this->a1 = new Q31(coefficientScaling * coefficients.A1, Q31::MaximumFractionalBits);
		this->a2 = new Q31(coefficientScaling * coefficients.A2, Q31::MaximumFractionalBits);
		this->b0 = new Q31(coefficientScaling * coefficients.B0, Q31::MaximumFractionalBits);
		this->b1 = new Q31(coefficientScaling * coefficients.B1, Q31::MaximumFractionalBits);
		this->b2 = new Q31(coefficientScaling * coefficients.B2, Q31::MaximumFractionalBits);

		this->channels = channels;
		this->x1 = new __int32[channels];
		this->x2 = new __int32[channels];
		this->y1 = new __int64[channels];
		this->y2 = new __int64[channels];
		for (__int32 channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->x2[channel] = 0;
			this->y1[channel] = 0;
			this->y2[channel] = 0;
		}
	}

	Biquad1Q31_32x64::~Biquad1Q31_32x64()
	{
		delete this->a1;
		delete this->a2;
		delete this->b0;
		delete this->b1;
		delete this->b2;

		delete this->x1;
		delete this->x2;
		delete this->y1;
		delete this->y2;
	}

	/// <summary>
	/// Process a single sample.  The sample must be in [ 0.25, 0.25 ) to guarantee the accumulator does not overflow and wrap around.
	/// </summary>
	void Biquad1Q31_32x64::Filter(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			__int32 channel = sample % this->channels;
			__int32 value = block[sample];
			__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] + *(this->b2) * this->x2[channel] - *(this->a1) * this->y1[channel] - *(this->a2) * this->y2[channel];
			this->x2[channel] = this->x1[channel];
			this->x1[channel] = value;
			this->y2[channel] = this->y1[channel];
			this->y1[channel] = accumulator << (this->postShift + 1);
			block[sample] = (__int32)(accumulator >> (Q31::MaximumFractionalBits - this->postShift));
		}
	}

	void Biquad1Q31_32x64::FilterReverse(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = maxSample - 1; sample >= offset; --sample)
		{
			__int32 channel = sample % this->channels;
			__int32 value = block[sample];
			__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] + *(this->b2) * this->x2[channel] - *(this->a1) * this->y1[channel] - *(this->a2) * this->y2[channel];
			this->x2[channel] = this->x1[channel];
			this->x1[channel] = value;
			this->y2[channel] = this->y1[channel];
			this->y1[channel] = accumulator << (this->postShift + 1);
			block[sample] = (__int32)(accumulator >> (Q31::MaximumFractionalBits - this->postShift));
		}
	}
}