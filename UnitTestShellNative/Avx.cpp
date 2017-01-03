#include "stdafx.h"
#include <chrono>
#include <immintrin.h>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include "Avx.h"
#include "StereoBiquad1Double128.h"
#include "StereoBiquad1Double256.h"
#include "StereoBiquad1FirstOrder1Double128.h"
#include "StereoBiquad2FirstOrder2Double256.h"

using namespace std::chrono;

namespace CrossTimeDsp::UnitTestShellNative
{
	Avx::Avx(__int32 samples, __int32 sampleBlockSizeInBytes)
	{
		this->samples = samples;
		this->sampleBlockSizeInBytes = sampleBlockSizeInBytes;
	}

	void Avx::Biquad2FirstOrder2Avx128(BiquadCoefficients biquad0Coefficients, FirstOrderCoefficients firstOrder0Coefficients, BiquadCoefficients biquad1Coefficients, FirstOrderCoefficients firstOrder1Coefficients, unique_ptr<list<double*>> &blocks)
	{
		StereoBiquad1FirstOrder1Double128 thirdOrder0(biquad0Coefficients, firstOrder0Coefficients);
		StereoBiquad1FirstOrder1Double128 thirdOrder1(biquad1Coefficients, firstOrder1Coefficients);
		vector<IFilter<double>*> filters;
		filters.push_back(&thirdOrder0);
		filters.push_back(&thirdOrder1);
		this->FilterBlocks(filters, blocks);
	}

	void Avx::Biquad2FirstOrder2Avx256(BiquadCoefficients biquad0Coefficients, FirstOrderCoefficients firstOrder0Coefficients, BiquadCoefficients biquad1Coefficients, FirstOrderCoefficients firstOrder1Coefficients, unique_ptr<list<double*>> &blocks)
	{
		StereoBiquad2FirstOrder2Double256 sixthOrder(biquad0Coefficients, firstOrder0Coefficients, biquad1Coefficients, firstOrder1Coefficients);
		vector<IFilter<double>*> filters;
		filters.push_back(&sixthOrder);
		this->FilterBlocks(filters, blocks);
	}

	void Avx::Biquad1Avx128(BiquadCoefficients biquadCoefficients, unique_ptr<list<double*>> &blocks)
	{
		StereoBiquad1Double128 biquad(biquadCoefficients);
		vector<IFilter<double>*> filters;
		filters.push_back(&biquad);
		this->FilterBlocks(filters, blocks);
	}

	void Avx::Biquad1Avx256(BiquadCoefficients biquadCoefficients, unique_ptr<list<double*>> &blocks)
	{
		StereoBiquad1Double256 biquad(biquadCoefficients);
		vector<IFilter<double>*> filters;
		filters.push_back(&biquad);
		this->FilterBlocks(filters, blocks);
	}

	unique_ptr<list<double*>> Avx::FillData()
	{
		const __int32 doublesPerBlock = sampleBlockSizeInBytes / sizeof(double);
		const __int32 blocksToFill = samples / doublesPerBlock;

		unique_ptr<list<double*>> blocks(new list<double*>());
		bool increment = true;
		double value = -10000;
		for (__int32 blockIndex = 0; blockIndex < blocksToFill; ++blockIndex)
		{
			double* block = reinterpret_cast<double*>(_aligned_malloc(sampleBlockSizeInBytes, alignof(__m256d)));
			for (__int32 sample = 0; sample < doublesPerBlock; ++sample)
			{
				if (increment)
				{
					++value;
				}
				else
				{
					--value;
				}
				if (value > 10000)
				{
					increment = false;
					value = 9999;
				}
				else if (value < -10000)
				{
					increment = true;
					value = -9999;
				}
				*(block + sample) = value;
			}

			blocks->push_back(block);
		}

		return blocks;
	}

	void Avx::FilterBlocks(vector<IFilter<double>*> &filters, unique_ptr<list<double*>> &blocks)
	{
		const __int32 filterBlocksPerSampleBlock = this->sampleBlockSizeInBytes / Constant::FilterBlockSizeInBytes;
		const __int32 samplesPerFilterBlock = Constant::FilterBlockSizeInBytes / sizeof(double);
		for (list<double*>::iterator blockIterator = blocks->begin(); blockIterator != blocks->end(); ++blockIterator)
		{
			for (int filterBlock = 0; filterBlock < filterBlocksPerSampleBlock; ++filterBlock)
			{
				for (int filter = 0; filter < filters.size(); ++filter)
				{
					filters.at(filter)->Filter(*blockIterator, filterBlock * samplesPerFilterBlock);
				}
			}
		}
	}

	void Avx::FilterBlocks(__int32 order, bool printTiming)
	{
		unique_ptr<list<double*>> blocks = this->FillData();

		double lowCrossoverW0 = 200.0 / 41000.0;
		double lr6InverseAllpassAlpha = sin(lowCrossoverW0) / (2.0 * Constant::Linearization::LR6InverseAllpassQ);
		BiquadCoefficients biquad0Coefficients = BiquadCoefficients::Create(FilterTypeNative::Allpass, lowCrossoverW0, lr6InverseAllpassAlpha, 0.0);
		FirstOrderCoefficients firstOrder0Coefficients = FirstOrderCoefficients::Create(FilterTypeNative::Allpass, lowCrossoverW0, 0.0);

		double highCrossoverW0 = 1700.0 / 41000.0;
		BiquadCoefficients biquad1Coefficients = BiquadCoefficients::Create(FilterTypeNative::Allpass, highCrossoverW0, lr6InverseAllpassAlpha, 0.0);
		FirstOrderCoefficients firstOrder1Coefficients = FirstOrderCoefficients::Create(FilterTypeNative::Allpass, highCrossoverW0, 0.0);

		high_resolution_clock::time_point start = high_resolution_clock::now();
		switch (order)
		{
		case 2:
			this->Biquad1Avx256(biquad0Coefficients, blocks);
			break;
		case 6:
			this->Biquad2FirstOrder2Avx256(biquad0Coefficients, firstOrder0Coefficients, biquad1Coefficients, firstOrder1Coefficients, blocks);
			break;
		default:
			throw new invalid_argument("Unhandled filter order " + to_string(order) + ".");
		}
		high_resolution_clock::time_point avx128complete = high_resolution_clock::now();
		switch (order)
		{
		case 2:
			this->Biquad1Avx128(biquad0Coefficients, blocks);
			break;
		case 6:
			this->Biquad2FirstOrder2Avx128(biquad0Coefficients, firstOrder0Coefficients, biquad1Coefficients, firstOrder1Coefficients, blocks);
			break;
		default:
			throw new invalid_argument("Unhandled filter order " + to_string(order) + ".");
		}
		high_resolution_clock::time_point avx129complete = high_resolution_clock::now();

		for (list<double*>::iterator blockIterator = blocks->begin(); blockIterator != blocks->end(); ++blockIterator)
		{
			_aligned_free(*blockIterator);
		}

		double avx256milliseconds = 1E-6 * duration_cast<nanoseconds>(avx128complete - start).count();
		double avx129milliseconds = 1E-6 * duration_cast<nanoseconds>(avx129complete - avx128complete).count();
		double avx256speedup = avx129milliseconds / avx256milliseconds;
		cout.setf(ios::fixed, ios::floatfield);
		cout.precision(2);
		if (printTiming)
		{
			cout << "AVX 256 " << avx256speedup << "x faster than AVX 128 (" << avx256milliseconds << " versus " << avx129milliseconds << " ms)" << endl;
		}
		else
		{
			cout << avx256speedup << endl;
		}
	}
}