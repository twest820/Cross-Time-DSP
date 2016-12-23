#include "stdafx.h"
#include <initializer_list>
#include "BiquadQ31.h"

namespace CrossTimeDsp::Dsp
{
	BiquadQ31::BiquadQ31(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels)
	{
		a1 /= a0;
		a2 /= a0;
		b0 /= a0;
		b1 /= a0;
		b2 /= a0;
		this->fractionalBitsInCoefficients = Q31::GetOptimalNumberOfFractionalBits(std::initializer_list<double>({ a1, a2, b0, b1, b2 }));
		this->a1 = new Q31(a1, this->fractionalBitsInCoefficients);
		this->a2 = new Q31(a2, this->fractionalBitsInCoefficients);
		this->b0 = new Q31(b0, this->fractionalBitsInCoefficients);
		this->b1 = new Q31(b1, this->fractionalBitsInCoefficients);
		this->b2 = new Q31(b2, this->fractionalBitsInCoefficients);

		this->channels = channels;
		this->x1 = new int[channels];
		this->x2 = new int[channels];
		this->y1 = new int[channels];
		this->y2 = new int[channels];
		for (int channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->x2[channel] = 0;
			this->y1[channel] = 0;
			this->y2[channel] = 0;
		}
	}

	BiquadQ31::~BiquadQ31()
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

	void BiquadQ31::Filter(__int32* block, __int32 offset)
	{
		int maxSample = offset + Constant::FilterBlockSizeInInt32s;
		if (this->channels == 2)
		{
			for (__int32 sample = offset; sample < maxSample; sample += 2)
			{
				__int32 value0 = block[sample];
				__int64 accumulator0 = *(this->b0) * value0 + *(this->b1) * this->x1[0] + *(this->b2) * this->x2[0] - *(this->a1) * this->y1[0] - *(this->a2) * this->y2[0];
				this->x2[0] = this->x1[0];
				this->x1[0] = value0;
				this->y2[0] = this->y1[0];
				this->y1[0] = (int)(accumulator0 >> this->fractionalBitsInCoefficients);
				block[sample] = this->y1[0];

				__int32 value1 = block[sample + 1];
				__int64 accumulator1 = *(this->b0) * value1 + *(this->b1) * this->x1[1] + *(this->b2) * this->x2[1] - *(this->a1) * this->y1[1] - *(this->a2) * this->y2[1];
				this->x2[1] = this->x1[1];
				this->x1[1] = value1;
				this->y2[1] = this->y1[1];
				this->y1[1] = (int)(accumulator1 >> this->fractionalBitsInCoefficients);
				block[sample + 1] = this->y1[1];
			}
		}
		else
		{
			for (__int32 sample = offset; sample < maxSample; ++sample)
			{
				__int32 channel = sample % this->channels;
				__int32 value = block[sample];
				__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] + *(this->b2) * this->x2[channel] - *(this->a1) * this->y1[channel] - *(this->a2) * this->y2[channel];
				this->x2[channel] = this->x1[channel];
				this->x1[channel] = value;
				this->y2[channel] = this->y1[channel];
				this->y1[channel] = (int)(accumulator >> this->fractionalBitsInCoefficients);
				block[sample] = this->y1[channel];
			}
		}
	}

	void BiquadQ31::FilterReverse(__int32* block, __int32 offset)
	{
		int maxSample = offset + Constant::FilterBlockSizeInInt32s;
		if (this->channels == 2)
		{
			for (__int32 sample = maxSample - 2; sample >= offset; sample -= 2)
			{
				__int32 value0 = block[sample];
				__int64 accumulator0 = *(this->b0) * value0 + *(this->b1) * this->x1[0] + *(this->b2) * this->x2[0] - *(this->a1) * this->y1[0] - *(this->a2) * this->y2[0];
				this->x2[0] = this->x1[0];
				this->x1[0] = value0;
				this->y2[0] = this->y1[0];
				this->y1[0] = (int)(accumulator0 >> this->fractionalBitsInCoefficients);
				block[sample] = this->y1[0];

				__int32 value1 = block[sample + 1];
				__int64 accumulator1 = *(this->b0) * value1 + *(this->b1) * this->x1[1] + *(this->b2) * this->x2[1] - *(this->a1) * this->y1[1] - *(this->a2) * this->y2[1];
				this->x2[1] = this->x1[1];
				this->x1[1] = value1;
				this->y2[1] = this->y1[1];
				this->y1[1] = (int)(accumulator1 >> this->fractionalBitsInCoefficients);
				block[sample + 1] = this->y1[1];
			}
		}
		else
		{
			for (__int32 sample = maxSample - 1; sample >= offset; --sample)
			{
				__int32 channel = sample % this->channels;
				__int32 value = block[sample];
				__int64 accumulator = *(this->b0) * value + *(this->b1) * this->x1[channel] + *(this->b2) * this->x2[channel] - *(this->a1) * this->y1[channel] - *(this->a2) * this->y2[channel];
				this->x2[channel] = this->x1[channel];
				this->x1[channel] = value;
				this->y2[channel] = this->y1[channel];
				this->y1[channel] = (int)(accumulator >> this->fractionalBitsInCoefficients);
				block[sample] = this->y1[channel];
			}
		}
	}
}