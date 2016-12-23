#include "stdafx.h"
#include "Constant.h"
#include "GainQ31.h"

namespace CrossTimeDsp::Dsp
{
	GainQ31::GainQ31(double scaleFactor)
	{
		this->scaleFactor = new Q31(scaleFactor);
	}

	GainQ31::~GainQ31()
	{
		delete this->scaleFactor;
	}

	void GainQ31::Filter(__int32* block, __int32 offset)
	{
		__int32 maxSample = offset + Constant::FilterBlockSizeInInt32s;
		for (__int32 sample = offset; sample < maxSample; ++sample)
		{
			block[sample] = (__int32)((*(this->scaleFactor) * block[sample]) >> this->scaleFactor->FractionalBits());
		}
	}

	void GainQ31::FilterReverse(__int32* block, __int32 offset)
	{
		this->Filter(block, offset);
	}
}
