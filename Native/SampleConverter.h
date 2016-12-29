#pragma once
namespace CrossTimeDsp::Dsp
{
	private class SampleConverter
	{
	public:
		static void DoubleToQ15(double* doubles, __int16* int16s, __int32 samples);
		static void DoubleToQ23(double* doubles, unsigned __int8* bytes, __int32 samples);
		static void DoubleToQ31(double* doubles, __int32* int32s, __int32 samples);
		static void Q15ToDouble(__int16* int16s, double* doubles, __int32 samples);
		static void Q15ToQ31(__int16* int16s, __int32* int32s, __int32 samples);
		static void Q23ToDouble(unsigned __int8* bytes, double* doubles, __int32 samples);
		static void Q23ToQ31(unsigned __int8* bytes, __int32* int32s, __int32 samples);
		static void Q31ToDouble(__int32* int32s, double* doubles, __int32 samples);
		static void Q31ToQ15(__int32* int32s, __int16* int16s, __int32 samples);
		static void Q31ToQ23(__int32* int32s, unsigned __int8* bytes, __int32 samples);

	private:
		static __m128i Int24Max128;
		static __m256i Int24Max256;
		static __m128i Int24Min128;
		static __m256i Int24Min256;
		static __m128i ShuffleInt24ToInt32_128;
		static __m128i ShuffleInt32ToInt24_128;

		static class Initializer
		{
		public:
			Initializer();
		} InitializerInstance;
	};
}

