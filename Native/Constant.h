#pragma once

namespace CrossTimeDsp::Dsp {

	public class Constant
	{
	public:
		// size of sample block processed by an IFilter<TSample> implementation
		static const __int32 FilterBlockSizeInBytes = 36 * 1024;
		static const __int32 FilterBlockSizeInDoubles = Constant::FilterBlockSizeInBytes / sizeof(double);
		static const __int32 FilterBlockSizeInInt32s = Constant::FilterBlockSizeInBytes / sizeof(__int32);

		/// <summary>
		/// Shift factor for converting between 16 bit audio samples and Q0.31 on [ 0.25, 0.25 ).  All 16 bits in the samples are 
		/// assumed to be used.
		/// </summary>
		// TestConstant.Q15ToQ31TruncationErrorIncrease needs to be synchronized with this value.
		static const __int32 ShiftBetween16BitSamplesAndQ31 = 14;

		/// <summary>
		/// Shift factor for converting between 24 bit audio samples and Q0.31 on [ 0.25, 0.25 ).  All 24 bits in the samples are 
		/// assumed to be used.
		/// </summary>
		// TestConstant.Q23ToQ31TruncationErrorIncrease needs to be synchronized with this value.
		static const __int32 ShiftBetween24BitSamplesAndQ31 = 6;

		class Int24
		{
		public:
			static const int MaxValue = 8388607;
			static constexpr unsigned __int8 MaxValueBytes[3] = { 0xff, 0xff, 0x7f };
			static const int MinValue = -8388608;
			static constexpr unsigned __int8 MinValueBytes[3] = { 0x00, 0x00, 0x80 };
		};
	};
};
