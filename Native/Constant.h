#pragma once
#include <immintrin.h>

namespace CrossTimeDsp::Dsp {

	class __declspec(dllexport) Constant
	{
	public:
		// size of sample block processed by an IFilter<TSample> implementation
		// Must be a multiple of 128 bytes for SSE4 sample conversion from double to Int24.
		// Must be a multiple of 256 bytes for AVX2 sample conversion from Int16 to double.
		//	                               for AVX2 sample conversion from double to Int24.
		// Interesting sizes for tuning are typically 8 and 12k.
		static const __int32 FilterBlockSizeInBytes = 12 * 1024;
		static const __int32 FilterBlockSizeInDoubles = Constant::FilterBlockSizeInBytes / sizeof(double);
		static const __int32 FilterBlockSizeInInt32s = Constant::FilterBlockSizeInBytes / sizeof(__int32);

		/// <summary>
		/// Shift factor for converting between 16 bit audio samples and Q0.31 on [ 0.25, 0.25 ).  All 16 bits in the samples are 
		/// assumed to be used.
		/// </summary>
		// TestConstant.ShiftBetween16BitSamplesAndQ31 needs to be synchronized with this value.
		static const __int32 ShiftBetween16BitSamplesAndQ31 = 14;

		/// <summary>
		/// Shift factor for converting between 24 bit audio samples and Q0.31 on [ 0.25, 0.25 ).  All 24 bits in the samples are 
		/// assumed to be used.
		/// </summary>
		// TestConstant.ShiftBetween24BitSamplesAndQ31 needs to be synchronized with this value.
		static const __int32 ShiftBetween24BitSamplesAndQ31 = 6;

		static const bool Simd256SampleConversionEnabled = false;
		static const int Simd256FilterOrderRequired = 8;
		static const bool StereoOptimizationEnabled = true;

		class Int24
		{
		public:
			static const int LeftShiftFromInt24ToInt32 = 8;
			static const int RightShiftFromInt32ToQ31 = Int24::LeftShiftFromInt24ToInt32 - Constant::ShiftBetween24BitSamplesAndQ31;

			static const __int32 MaxValue = 8388607;
			static const __int32 MinValue = -8388608;
		};

		class Linearization
		{
		public:
			static constexpr double LR6InverseAllpassQ = 1.0;
			static constexpr double HalfRoot2 = 0.707106781186547524401;
		};

		class Simd128
		{
		public:
			static const int DeinterleaveInterleave4x32 = _MM_SHUFFLE(3, 1, 2, 0);
			static const int PermuteHighAndLow2x64 = _MM_SHUFFLE(1, 0, 3, 2);
			static const int PermuteHighAndLow4x32 = _MM_SHUFFLE(2, 3, 0, 1);
			static const int MergeLowAndHigh2x64 = 0xf0;
			static const int MergeLowAndHigh4x32 = 0xcc;
		};

		class Simd256
		{
		public:
			static const int ExchangeLanes = _MM_SHUFFLE2(0, 1);
			static const int ExtractUpper = 1;
			static const int PermuteHighToLowAndLowToHigh2x128 = 0x21;
			static const int InsertHigh = 1;
			static const int InsertLow = 0;
		};
	};
};
