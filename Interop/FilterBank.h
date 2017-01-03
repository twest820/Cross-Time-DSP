#pragma once
#include <vector>
#include "FilterPrecision.h"
#include "FilterType.h"
#include "IFilter.h"
#include "SampleBlock.h"
#include "SampleType.h"

using namespace System;

namespace CrossTimeDsp::Dsp
{
	public ref class FilterBank
	{
	private:
		initonly __int32 channels;
		std::vector<IFilter<double>*>* doubleFilters;
		std::vector<IFilter<__int32>*>* intFilters;
		__int64 filterTime;
		initonly __int32 fs;
		initonly FilterPrecision precision;
		initonly double q31_32x64_Threshold;
		initonly double q31_64x64_Threshold;
		initonly bool timingAvailable;
		__int64 toDataPathTime;
		__int64 toOutputTime;

	public:
		FilterBank(FilterPrecision precision, __int32 fs, __int32 channels, double q31_32x64_Threshold, double q31_64x64_Threshold);
		!FilterBank();
		~FilterBank();

		property __int32 FilterCount
		{
			__int32 get() { return (__int32)(this->doubleFilters->size() + this->intFilters->size()); }
		}

		property TimeSpan FilterTime
		{
			TimeSpan get() { return TimeSpan::FromTicks(this->filterTime); }
		}

		property bool TimingAvailable
		{
			bool get() { return this->timingAvailable; }
		}

		property TimeSpan ToDataPathTime
		{
			TimeSpan get() { return TimeSpan::FromTicks(this->toDataPathTime); }
		}

		property TimeSpan ToOutputTime
		{
			TimeSpan get() { return TimeSpan::FromTicks(this->toOutputTime); }
		}

		void AddBiquad(FilterType type, double f0, double q, double gainInDB);
		void AddFirstOrder(FilterType type, double f0, double gainInDB);
		void AddGain(double gainInDb);
		void AddThirdOrder(FilterType biquadType, double biquadF0, double biquadQ, double biquadGainInDB, FilterType firstOrderType, double firstOrderF0, double firstOrderGainInDB);
		void AddThreeWayLinearization(double lowCrossover, double highCrossover, double wooferRolloff, double midRolloff, double gainInDB);
		SampleBlock^ Filter(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType, SampleBlock^% recirculatingDataPathBlock);
		SampleBlock^ FilterReverse(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType, SampleBlock^% recirculatingDataPathBlock);

	private:
		SampleBlock^ FilterBank::ConvertOrRecirculateBlock(SampleBlock^ block, SampleType sampleType, SampleBlock^% recirculatingBlock);
		template <typename TSample> IFilter<TSample>* CreateBiquad(FilterType type, double f0, double q, double gainInDB);
		template <typename TSample> IFilter<TSample>* CreateFirstOrder(FilterType type, double f0, double gainInDB);
		template <typename TSample> IFilter<TSample>* CreateGain(double gainInDB);
		template <typename TSample> IFilter<TSample>* CreateThirdOrder(FilterType biquadType, double biquadF0, double biquadQ, double biquadGainInDB, FilterType firstOrderType, double firstOrderF0, double firstOrderGainInDB);
		template <typename TSample> std::vector<IFilter<TSample>*> CreateThreeWayLinearization(double lowCrossover, double highCrossover, double wooferRolloff, double midRolloff, double gainInDB);
		template <typename TSample> IFilter<TSample>* CreateSixthOrder(FilterType biquad0Type, double biquad0F0, double biquad0Q, double biquad0GainInDB, FilterType firstOrder0Type, double firstOrder0F0, double firstOrder0GainInDB,
																	   FilterType biquad1Type, double biquad1F0, double biquad1Q, double biquad1GainInDB, FilterType firstOrder1Type, double firstOrder1F0, double firstOrder1GainInDB);

		template <typename TSample> void Filter(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 blockLength, __int32 filterBlockSizeInSamples);
		template <typename TSample> void FilterReverse(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 count, __int32 filterBlockSizeInSamples);

		double GetAlpha(double w0, double q);
		double GetW0(double f0);
		FilterPrecision MaybeResolveAdaptiveFilterPrecision(double f0);
	};
}

