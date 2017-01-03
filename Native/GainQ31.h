#pragma once
#include "IFilter.h"
#include "Q31.h"

namespace CrossTimeDsp::Dsp
{
	class __declspec(dllexport) GainQ31 : public IFilter<__int32>
	{
	private:
		Q31* scaleFactor;

	public:
		GainQ31(double scaleFactor);
		~GainQ31();

		virtual void Filter(__int32* block, __int32 offset);
		virtual void FilterReverse(__int32* block, __int32 offset);
	};
}

