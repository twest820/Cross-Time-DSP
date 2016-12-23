#include "stdafx.h"
#include "BiquadQ31_64x64.h"
#include "Constant.h"
#include "Q31.h"

using namespace System;

namespace CrossTimeDsp::Dsp
{
	BiquadQ31_64x64::BiquadQ31_64x64(double b0, double b1, double b2, double a0, double a1, double a2, __int32 channels)
	{
		a1 /= a0;
		a2 /= a0;
		b0 /= a0;
		b1 /= a0;
		b2 /= a0;
		// scale coefficients into Q0.31 so that state variables can be stored in Q0.63 for maximum precision
		// the post shift is used to undo the scaling at output
		this->postShift = Q63::GetQ31_64x64_PostShift(std::initializer_list<double>({ a1, a2, b0, b1, b2 }));
		double coefficientScaling = Math::Pow(2.0, -this->postShift);
		this->a1 = new Q63(coefficientScaling * a1, Q63::MaximumFractionalBits);
		this->a2 = new Q63(coefficientScaling * a2, Q63::MaximumFractionalBits);
		this->b0 = new Q63(coefficientScaling * b0, Q63::MaximumFractionalBits);
		this->b1 = new Q63(coefficientScaling * b1, Q63::MaximumFractionalBits);
		this->b2 = new Q63(coefficientScaling * b2, Q63::MaximumFractionalBits);

		this->channels = channels;
		this->x1 = new __int32[channels];
		this->x2 = new __int32[channels];
		this->y1 = new __int64[channels];
		this->y2 = new __int64[channels];
		for (int channel = 0; channel < channels; ++channel)
		{
			this->x1[channel] = 0;
			this->x2[channel] = 0;
			this->y1[channel] = 0;
			this->y2[channel] = 0;
		}
	}

	BiquadQ31_64x64::~BiquadQ31_64x64()
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
	/// Samples must be in [ 0.25, 0.25 ) to guarantee the accumulator does not overflow and wrap around.
	/// </summary>
	void BiquadQ31_64x64::Filter(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		if (this->channels == 2)
		{
			for (__int32 sample = offset; sample < maxSample; sample += 2)
			{
				__int32 value0 = block[sample];
				__int64 accumulator0 = *(this->b0) * value0 + *(this->b1) * this->x1[0] + *(this->b2) * this->x2[0] - *(this->a1) * this->y1[0] - *(this->a2) * this->y2[0];
				this->x2[0] = this->x1[0];
				this->x1[0] = value0;
				this->y2[0] = this->y1[0];
				this->y1[0] = accumulator0 << (this->postShift + 1);
				block[sample] = (__int32)(accumulator0 >> (Q31::MaximumFractionalBits - this->postShift));

				__int32 value1 = block[sample + 1];
				__int64 accumulator1 = *(this->b0) * value1 + *(this->b1) * this->x1[1] + *(this->b2) * this->x2[1] - *(this->a1) * this->y1[1] - *(this->a2) * this->y2[1];
				this->x2[1] = this->x1[1];
				this->x1[1] = value1;
				this->y2[1] = this->y1[1];
				this->y1[1] = accumulator1 << (this->postShift + 1);
				block[sample + 1] = (__int32)(accumulator1 >> (Q31::MaximumFractionalBits - this->postShift));
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
				this->y1[channel] = accumulator << (this->postShift + 1);
				// output conversion is from a Q63 accumulator to Q31, so reference Q31::MaximumFractionalBits for the shift
				block[sample] = (__int32)(accumulator >> (Q31::MaximumFractionalBits - this->postShift));
			}
		}
	}

	void BiquadQ31_64x64::FilterReverse(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		if (this->channels == 2)
		{
			for (__int32 sample = maxSample - 2; sample >= offset; sample -= 2)
			{
				__int32 value0 = block[sample];
				__int64 accumulator0 = *(this->b0) * value0 + *(this->b1) * this->x1[0] + *(this->b2) * this->x2[0] - *(this->a1) * this->y1[0] - *(this->a2) * this->y2[0];
				this->x2[0] = this->x1[0];
				this->x1[0] = value0;
				this->y2[0] = this->y1[0];
				this->y1[0] = accumulator0 << (this->postShift + 1);
				block[sample] = (__int32)(accumulator0 >> (Q31::MaximumFractionalBits - this->postShift));

				__int32 value1 = block[sample + 1];
				__int64 accumulator1 = *(this->b0) * value1 + *(this->b1) * this->x1[1] + *(this->b2) * this->x2[1] - *(this->a1) * this->y1[1] - *(this->a2) * this->y2[1];
				this->x2[1] = this->x1[1];
				this->x1[1] = value1;
				this->y2[1] = this->y1[1];
				this->y1[1] = accumulator1 << (this->postShift + 1);
				block[sample + 1] = (__int32)(accumulator1 >> (Q31::MaximumFractionalBits - this->postShift));
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
				this->y1[channel] = accumulator << (this->postShift + 1);
				block[sample] = (__int32)(accumulator >> (Q31::MaximumFractionalBits - this->postShift));
			}
		}
	}
}
