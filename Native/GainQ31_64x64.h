#pragma once
#include "IFilter.h"
#include "Q63.h"

namespace CrossTimeDsp::Dsp
{
	private class GainQ31_64x64 : IFilter<__int32>
	{
	private:
		Q63* scaleFactor;

	public:
		GainQ31_64x64(double scaleFactor);
		~GainQ31_64x64();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}