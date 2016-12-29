#include "stdafx.h"
#include <initializer_list>
#include "Biquad1Q31.h"

namespace CrossTimeDsp::Dsp
{
	Biquad1Q31::Biquad1Q31(BiquadCoefficients coefficients, __int32 channels)
	{
		this->fractionalBitsInCoefficients = Q31::GetOptimalNumberOfFractionalBits(std::initializer_list<double>({ coefficients.A1, coefficients.A2, coefficients.B0, coefficients.B1, coefficients.B2 }));
		this->a1 = new Q31(coefficients.A1, this->fractionalBitsInCoefficients);
		this->a2 = new Q31(coefficients.A2, this->fractionalBitsInCoefficients);
		this->b0 = new Q31(coefficients.B0, this->fractionalBitsInCoefficients);
		this->b1 = new Q31(coefficients.B1, this->fractionalBitsInCoefficients);
		this->b2 = new Q31(coefficients.B2, this->fractionalBitsInCoefficients);

		this->channels = channels;
		this->x1 = new __int32[channels];
		this->x2 = new __int32[channels];
		this->y1 = new __int32[channels];
		this->y2 = new __int32[channels];
		for (__int32 channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->x2[channel] = 0;
			this->y1[channel] = 0;
			this->y2[channel] = 0;
		}
	}

	Biquad1Q31::~Biquad1Q31()
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

	void Biquad1Q31::Filter(__int32* block, __int32 offset)
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
			this->y1[channel] = (__int32)(accumulator >> this->fractionalBitsInCoefficients);
			block[sample] = this->y1[channel];
		}
	}

	void Biquad1Q31::FilterReverse(__int32* block, __int32 offset)
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
			this->y1[channel] = (__int32)(accumulator >> this->fractionalBitsInCoefficients);
			block[sample] = this->y1[channel];
		}
	}
}