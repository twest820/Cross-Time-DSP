#include "stdafx.h"
#include <algorithm>
#include "Biquad1Double.h"
#include "Biquad1Q31.h"
#include "Biquad1Q31_32x64.h"
#include "Biquad1Q31_64x64.h"
#include "Constant.h"
#include "FilterBank.h"
#include "FirstOrder1Double.h"
#include "FirstOrder1Q31.h"
#include "FirstOrder1Q31_32x64.h"
#include "GainDouble.h"
#include "GainQ31.h"
#include "GainQ31_64x64.h"
#include "InstructionSet.h"
#include "StereoBiquad1Double.h"
#include "StereoBiquad1Q31.h"
#include "StereoBiquad1Q31_32x64.h"
#include "StereoBiquad1Q31_64x64.h"
#include "StereoFirstOrder1Double.h"
#include "StereoBiquad1Q31.h"
#include "StereoFirstOrder1Q31.h"
#include "StereoFirstOrder1Q31_32x64.h"
#include "StereoBiquad1FirstOrder1Double.h"
#include "Biquad1FirstOrder1Double.h"

using namespace System;
using namespace System::Diagnostics;

namespace CrossTimeDsp::Dsp
{
	FilterBank::FilterBank(FilterPrecision precision, __int32 fs, __int32 channels, double q31_32x64_Threshold, double q31_64x64_Threshold)
	{
		if (InstructionSet::Sse41() == false)
		{
			throw gcnew NotSupportedException("Sorry, Cross Time DSP requires SSE4.1.  Please try on a newer machine (2008 or later is probably sufficient) or use Cross Time DSP 0.5.0.0.");
		}
		if (fs < 1)
		{
			throw gcnew ArgumentOutOfRangeException("fs", "Sampling rate must be greater than zero.");
		}
		if (channels < 1)
		{
			throw gcnew ArgumentOutOfRangeException("channels", "There must be at least one channel.");
		}
		if (q31_32x64_Threshold <= 0.0)
		{
			throw gcnew ArgumentOutOfRangeException("q31_32x64_Threshold", "32x64 threshold must be greater than zero.");
		}
		if (q31_64x64_Threshold <= 0.0)
		{
			throw gcnew ArgumentOutOfRangeException("q31_32x64_Threshold", "64x64 threshold must be greater than zero.");
		}

		this->channels = channels;
		this->filterTime = 0;
		this->doubleFilters = new std::vector<IFilter<double>*>();
		this->intFilters = new std::vector<IFilter<__int32>*>();
		this->fs = fs;
		this->precision = precision;
		this->q31_32x64_Threshold = q31_32x64_Threshold;
		this->q31_64x64_Threshold = q31_64x64_Threshold;
		this->timingAvailable = false;
		this->toDataPathTime = 0;
		this->toOutputTime = 0;
	#ifdef FILTER_BANK_INTERNAL_TIMING
		this->timingAvailable = true;
	#endif
	}

	FilterBank::!FilterBank()
	{
		for (__int32 filter = 0; filter < this->doubleFilters->size(); ++filter)
		{
			delete this->doubleFilters->at(filter);
		}
		delete this->doubleFilters;
		this->doubleFilters = nullptr;

		for (__int32 filter = 0; filter < this->intFilters->size(); ++filter)
		{
			delete this->intFilters->at(filter);
		}
		delete this->intFilters;
		this->intFilters = nullptr;
	}

	FilterBank::~FilterBank()
	{
		FilterBank::!FilterBank();
	}

	void FilterBank::AddBiquad(FilterType type, double f0, double q, double gainInDB)
	{
		if (this->precision == FilterPrecision::Double)
		{
			this->doubleFilters->push_back(this->CreateBiquad<double>(type, f0, q, gainInDB));
		}
		else
		{
			this->intFilters->push_back(this->CreateBiquad<__int32>(type, f0, q, gainInDB));
		}
	}

	void FilterBank::AddFirstOrder(FilterType type, double f0, double gainInDB)
	{
		if (this->precision == FilterPrecision::Double)
		{
			this->doubleFilters->push_back(this->CreateFirstOrder<double>(type, f0, gainInDB));
		}
		else
		{
			this->intFilters->push_back(this->CreateFirstOrder<__int32>(type, f0, gainInDB));
		}
	}

	void FilterBank::AddGain(double gainInDb)
	{
		if (this->precision == FilterPrecision::Double)
		{
			this->doubleFilters->push_back(this->CreateGain<double>(gainInDb));
		}
		else
		{
			this->intFilters->push_back(this->CreateGain<__int32>(gainInDb));
		}
	}

	void FilterBank::AddThirdOrder(FilterType biquadType, double biquadF0, double biquadQ, double biquadGainInDB, FilterType firstOrderType, double firstOrderF0, double firstOrderGainInDB)
	{
		if (this->precision == FilterPrecision::Double)
		{
			this->doubleFilters->push_back(this->CreateThirdOrder<double>(biquadType, biquadF0, biquadQ, biquadGainInDB, firstOrderType, firstOrderF0, firstOrderGainInDB));
		}
		else
		{
			this->intFilters->push_back(this->CreateThirdOrder<__int32>(biquadType, biquadF0, biquadQ, biquadGainInDB, firstOrderType, firstOrderF0, firstOrderGainInDB));
		}
	}

	void FilterBank::AddThreeWayLinearization(double lowCrossover, double highCrossover, double wooferRolloff, double midRolloff, double gainInDB)
	{
		if (this->precision == FilterPrecision::Double)
		{
			std::vector<IFilter<double>*> filters = this->CreateThreeWayLinearization<double>(lowCrossover, highCrossover, wooferRolloff, midRolloff, gainInDB);
			for (__int32 filter = 0; filter < filters.size(); ++filter)
			{
				this->doubleFilters->push_back(filters.at(filter));
			}
		}
		else
		{
			std::vector<IFilter<__int32>*> filters = this->CreateThreeWayLinearization<__int32>(lowCrossover, highCrossover, wooferRolloff, midRolloff, gainInDB);
			for (__int32 filter = 0; filter < filters.size(); ++filter)
			{
				this->intFilters->push_back(filters.at(filter));
			}
		}
	}

	SampleBlock^ FilterBank::ConvertOrRecirculateBlock(SampleBlock^ block, SampleType sampleType, SampleBlock^% recirculatingBlock)
	{
		if (block->SampleType == sampleType)
		{
			return block;
		}

		if (recirculatingBlock == nullptr)
		{
			// allocate data path block if no block is available
			return block->ConvertTo(sampleType);
		}
		if (recirculatingBlock->MaximumSizeInBytes < block->BytesPerSample(sampleType) * block->SamplesInUse)
		{
			// reallocate data path block if it's too small (typically this happens after the last block of a buffer is processed as the 
			// first block in a reverse time pass)
			delete recirculatingBlock;
			return block->ConvertTo(sampleType);
		}

		// reuse data path block
		block->ConvertTo(recirculatingBlock);
		return recirculatingBlock;
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateBiquad(FilterType type, double f0, double q, double gainInDB)
	{
		double w0 = this->GetW0(f0);
		double alpha = this->GetAlpha(w0, q);
		BiquadCoefficients coefficients = BiquadCoefficients::Create(type, w0, alpha, gainInDB);

		FilterPrecision precision = this->MaybeResolveAdaptiveFilterPrecision(f0);
		if (this->channels == 2 && Constant::StereoOptimizationEnabled)
		{
			switch (precision)
			{
			case FilterPrecision::Double:
				return (IFilter<TSample>*)new StereoBiquad1Double(coefficients);
			case FilterPrecision::Q31:
				return (IFilter<TSample>*)new StereoBiquad1Q31(coefficients);
			case FilterPrecision::Q31_32x64:
				return (IFilter<TSample>*)new StereoBiquad1Q31_32x64(coefficients);
			case FilterPrecision::Q31_64x64:
				return (IFilter<TSample>*)new StereoBiquad1Q31_64x64(coefficients);
			default:
				throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
			}
		}

		switch (precision)
		{
		case FilterPrecision::Double:
			return (IFilter<TSample>*)new Biquad1Double(coefficients, this->channels);
		case FilterPrecision::Q31:
			return (IFilter<TSample>*)new Biquad1Q31(coefficients, this->channels);
		case FilterPrecision::Q31_32x64:
			return (IFilter<TSample>*)new Biquad1Q31_32x64(coefficients, this->channels);
		case FilterPrecision::Q31_64x64:
			return (IFilter<TSample>*)new Biquad1Q31_64x64(coefficients, this->channels);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
		}
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateFirstOrder(FilterType type, double f0, double gainInDB)
	{
		double w0 = this->GetW0(f0);
		FirstOrderCoefficients coefficients = FirstOrderCoefficients::Create(type, w0, gainInDB);
		
		FilterPrecision precision = this->MaybeResolveAdaptiveFilterPrecision(f0);
		if (this->channels == 2 && Constant::StereoOptimizationEnabled)
		{
			switch (precision)
			{
			case FilterPrecision::Double:
				return (IFilter<TSample>*)new StereoFirstOrder1Double(coefficients);
			case FilterPrecision::Q31:
				return (IFilter<TSample>*)new StereoFirstOrder1Q31(coefficients);
			case FilterPrecision::Q31_32x64:
			case FilterPrecision::Q31_64x64:
				return (IFilter<TSample>*)new StereoFirstOrder1Q31_32x64(coefficients);
			default:
				throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
			}
		}

		switch (precision)
		{
		case FilterPrecision::Double:
			return (IFilter<TSample>*)new FirstOrder1Double(coefficients, this->channels);
		case FilterPrecision::Q31:
			return (IFilter<TSample>*)new FirstOrder1Q31(coefficients, this->channels);
		case FilterPrecision::Q31_32x64:
		case FilterPrecision::Q31_64x64:
			return (IFilter<TSample>*)new FirstOrder1Q31_32x64(coefficients, this->channels);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
		}
	}

	template <typename TSample>	IFilter<TSample>* FilterBank::CreateGain(double gainInDB)
	{
		double gain = Math::Pow(10.0, gainInDB / 20.0);
		switch (this->precision)
		{
		case FilterPrecision::Double:
			return (IFilter<TSample>*)new GainDouble(gain);
		case FilterPrecision::Q31:
		case FilterPrecision::Q31_32x64:
		case FilterPrecision::Q31Adaptive:
			return (IFilter<TSample>*)new GainQ31(gain);
		case FilterPrecision::Q31_64x64:
			return (IFilter<TSample>*)new GainQ31_64x64(gain);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", this->precision));
		}
	}

	template <typename TSample> IFilter<TSample>* FilterBank::CreateThirdOrder(FilterType biquadType, double biquadF0, double biquadQ, double biquadGainInDB, FilterType firstOrderType, double firstOrderF0, double firstOrderGainInDB)
	{
		double biquadW0 = this->GetW0(biquadF0);
		double biquadAlpha = this->GetAlpha(biquadW0, biquadQ);
		BiquadCoefficients biquadCoefficients = BiquadCoefficients::Create(biquadType, biquadW0, biquadAlpha, biquadGainInDB);

		double firstOrderW0 = this->GetW0(firstOrderF0);
		FirstOrderCoefficients firstOrderCoefficients = FirstOrderCoefficients::Create(firstOrderType, firstOrderW0, firstOrderGainInDB);

		FilterPrecision precision = this->MaybeResolveAdaptiveFilterPrecision(std::min(biquadF0, firstOrderF0));
		if (this->channels == 2 && Constant::StereoOptimizationEnabled)
		{
			switch (precision)
			{
			case FilterPrecision::Double:
				return (IFilter<TSample>*)new StereoBiquad1FirstOrder1Double(biquadCoefficients, firstOrderCoefficients);
			default:
				throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
			}
		}

		switch (precision)
		{
		case FilterPrecision::Double:
			return (IFilter<TSample>*)new Biquad1FirstOrder1Double(biquadCoefficients, firstOrderCoefficients, this->channels);
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled filter precision {0}.", precision));
		}
	}

	template <typename TSample> std::vector<IFilter<TSample>*> FilterBank::CreateThreeWayLinearization(double lowCrossover, double highCrossover, double wooferRolloff, double midRolloff, double gainInDB)
	{
		std::vector<IFilter<TSample>*> filters;
		filters.push_back(this->CreateThirdOrder<TSample>(FilterType::Allpass, midRolloff, Constant::Linearization::HalfRoot2, gainInDB, FilterType::Allpass, wooferRolloff, 0.0));
		filters.push_back(this->CreateThirdOrder<TSample>(FilterType::Allpass, lowCrossover, Constant::Linearization::LR6InverseAllpassQ, 0.0, FilterType::Allpass, lowCrossover, 0.0));
		filters.push_back(this->CreateThirdOrder<TSample>(FilterType::Allpass, highCrossover, Constant::Linearization::LR6InverseAllpassQ, 0.0, FilterType::Allpass, highCrossover, 0.0));
		return filters;
	}

	SampleBlock^ FilterBank::Filter(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType, SampleBlock^% recirculatingDataPathBlock)
	{
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime dataPathStart = DateTime::UtcNow;
	#endif
		SampleBlock^ dataPathBlock = this->ConvertOrRecirculateBlock(block, dataPathSampleType, recirculatingDataPathBlock);
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime dataPathStop = DateTime::UtcNow;
	#endif

		switch (dataPathSampleType)
		{
		case SampleType::Double:
			this->Filter<double>(this->doubleFilters, dataPathBlock->Doubles, dataPathBlock->DoubleSamples, Constant::FilterBlockSizeInDoubles);
			break;
		case SampleType::Int32:
			this->Filter<__int32>(this->intFilters, dataPathBlock->Int32s, dataPathBlock->Int32Samples, Constant::FilterBlockSizeInInt32s);
			break;
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled data path sample type {0}.", dataPathSampleType));
		}
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime filterStop = DateTime::UtcNow;
	#endif

		SampleBlock^ outputBlock;
		if (dataPathBlock->SampleType != outputSampleType)
		{
			outputBlock = dataPathBlock->ConvertTo(outputSampleType);
			if (block->SampleType != dataPathSampleType)
			{
				// if block conversion occurs on both input and output the data path block is scoped to this function and can be reused on the next iteration
				// Reuse avoid performance degradation from a many GB pileup of unused memory as the garbage collector doesn't keep up with the data path block
				// discard rate.  The slight overhead of going to the aligned memory system to discard blocks and retrieve them from its cache is also avoided.
				recirculatingDataPathBlock = dataPathBlock;
			}
		}
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime outputStop = DateTime::UtcNow;

		this->toDataPathTime += dataPathStop.Ticks - dataPathStart.Ticks;
		this->filterTime += filterStop.Ticks - dataPathStop.Ticks;
		this->toOutputTime += outputStop.Ticks - filterStop.Ticks;
	#endif
		return outputBlock;
	}

	template <typename TSample>	void FilterBank::Filter(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 blockLength, __int32 filterBlockSizeInSamples)
	{
		for (__int32 index = 0; index < blockLength; index += filterBlockSizeInSamples)
		{
			for (__int32 filter = 0; filter < filters->size(); ++filter)
			{
				filters->at(filter)->Filter(block, index);
			}
		}
	}

	SampleBlock^ FilterBank::FilterReverse(SampleBlock^ block, SampleType dataPathSampleType, SampleType outputSampleType, SampleBlock^% recirculatingDataPathBlock)
	{
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime dataPathStart = DateTime::UtcNow;
	#endif
		SampleBlock^ dataPathBlock = this->ConvertOrRecirculateBlock(block, dataPathSampleType, recirculatingDataPathBlock);
		dataPathBlock->ZeroUnused();
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime dataPathStop = DateTime::UtcNow;
	#endif

		__int32 count = 0;
		switch (dataPathSampleType)
		{
		case SampleType::Double:
			count = dataPathBlock->DoubleSamples;
			if (count % Constant::FilterBlockSizeInDoubles != 0)
			{
				count = (count / Constant::FilterBlockSizeInDoubles + 1) * Constant::FilterBlockSizeInDoubles;
			}
			Debug::Assert(count * sizeof(double) <= dataPathBlock->MaximumSizeInBytes, String::Format("Block allocation failure: maximum size of {0} bytes is not a multiple of the {1} byte filter block size.", dataPathBlock->MaximumSizeInBytes, Constant::FilterBlockSizeInBytes));
			this->FilterReverse<double>(this->doubleFilters, dataPathBlock->Doubles, count, Constant::FilterBlockSizeInDoubles);
			break;
		case SampleType::Int32:
			count = dataPathBlock->Int32Samples;
			if (count % Constant::FilterBlockSizeInInt32s != 0)
			{
				count = (count / Constant::FilterBlockSizeInInt32s + 1) * Constant::FilterBlockSizeInInt32s;
			}
			Debug::Assert(count * sizeof(__int32) <= dataPathBlock->MaximumSizeInBytes, String::Format("Block allocation failure: maximum size of {0} bytes is not a multiple of the {1} byte filter block size.", dataPathBlock->MaximumSizeInBytes, Constant::FilterBlockSizeInBytes));
			this->FilterReverse<__int32>(this->intFilters, dataPathBlock->Int32s, count, Constant::FilterBlockSizeInInt32s);
			break;
		default:
			throw gcnew NotSupportedException(String::Format("Unhandled data path sample type {0}.", dataPathSampleType));
		}
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime filterStop = DateTime::UtcNow;
	#endif

		SampleBlock^ outputBlock = dataPathBlock;
		if (dataPathBlock->SampleType != outputSampleType)
		{
			outputBlock = dataPathBlock->ConvertTo(outputSampleType);
			if (block->SampleType != dataPathSampleType)
			{
				// see remarks in Filter()
				recirculatingDataPathBlock = dataPathBlock;
			}
		}
	#ifdef FILTER_BANK_INTERNAL_TIMING
		DateTime outputStop = DateTime::UtcNow;

		this->toDataPathTime += dataPathStop.Ticks - dataPathStart.Ticks;
		this->filterTime += filterStop.Ticks - dataPathStop.Ticks;
		this->toOutputTime += outputStop.Ticks - filterStop.Ticks;
	#endif
		return outputBlock;
	}

	template <typename TSample>	void FilterBank::FilterReverse(std::vector<IFilter<TSample>*>* filters, TSample* block, __int32 count, __int32 filterBlockSizeInSamples)
	{
		for (__int32 index = count - filterBlockSizeInSamples; index >= 0; index -= filterBlockSizeInSamples)
		{
			for (__int32 filter = 0; filter < filters->size(); ++filter)
			{
				filters->at(filter)->FilterReverse(block, index);
			}
		}
	}

	double FilterBank::GetAlpha(double w0, double q)
	{
		return Math::Sin(w0) / (2.0 * q);
	}

	double FilterBank::GetW0(double f0)
	{
		Debug::Assert(f0 < 0.5 * this->fs, String::Format("Center frequency of {0}Hz exceeds Nyquist frequency of {1}kHz.", f0, 0.001 * this->fs));
		return 2.0 * Math::PI * f0 / this->fs;
	}

	FilterPrecision FilterBank::MaybeResolveAdaptiveFilterPrecision(double f0)
	{
		if (this->precision != FilterPrecision::Q31Adaptive)
		{
			return this->precision;
		}

		double normalizedCenterFrequency = f0 / this->fs;
		if (normalizedCenterFrequency > this->q31_32x64_Threshold)
		{
			return FilterPrecision::Q31;
		}
		else if (normalizedCenterFrequency > this->q31_64x64_Threshold)
		{
			return FilterPrecision::Q31_32x64;
		}
		else
		{
			return FilterPrecision::Q31_64x64;
		}
	}
}