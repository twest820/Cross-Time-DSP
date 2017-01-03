#pragma once
#include <list>
#include <memory>
#include <vector>
#include "BiquadCoefficients.h"
#include "FirstOrderCoefficients.h"
#include "IFilter.h"

using namespace CrossTimeDsp::Dsp;
using namespace std;

namespace CrossTimeDsp::UnitTestShellNative
{
	class Avx
	{
	private:
		__int32 sampleBlockSizeInBytes;
		__int32 samples;

	public:
		Avx(__int32 samples, __int32 sampleBlockSizeInBytes);
		void FilterBlocks(__int32 order, bool printTiming);

	private:
		void Biquad2FirstOrder2Avx128(BiquadCoefficients biquad0Coefficients, FirstOrderCoefficients firstOrder0Coefficients, BiquadCoefficients biquad1Coefficients, FirstOrderCoefficients firstOrder1Coefficients, unique_ptr<list<double*>> &blocks);
		void Biquad2FirstOrder2Avx256(BiquadCoefficients biquad0Coefficients, FirstOrderCoefficients firstOrder0Coefficients, BiquadCoefficients biquad1Coefficients, FirstOrderCoefficients firstOrder1Coefficients, unique_ptr<list<double*>> &blocks);
		void Biquad1Avx128(BiquadCoefficients biquadCoefficients, unique_ptr<list<double*>> &blocks);
		void Biquad1Avx256(BiquadCoefficients biquadCoefficients, unique_ptr<list<double*>> &blocks);
		unique_ptr<list<double*>> FillData();
		void FilterBlocks(vector<IFilter<double>*> &filters, unique_ptr<list<double*>> &blocks);
	};
}

