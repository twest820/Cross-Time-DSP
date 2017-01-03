using System;

namespace CrossTimeDsp.Dsp
{
    internal static class SampleTypeExtensions
    {
        public static int BitsPerSample(this SampleType sampleType)
        {
            return 8 * sampleType.BytesPerSample();
        }

        public static int BytesPerSample(this SampleType sampleType)
        {
            switch (sampleType)
            {
                case SampleType.Double:
                    return 8;
                case SampleType.Int16:
                    return 2;
                case SampleType.Int24:
                    return 3;
                case SampleType.Int32:
                    return 4;
                default:
                    throw new NotSupportedException(String.Format("Sample type {0} is not supported.", sampleType));
            }
        }

        public static SampleType FromBitsPerSample(int bitsPerSample)
        {
            switch (bitsPerSample)
            {
                case 16:
                    return SampleType.Int16;
                case 24:
                    return SampleType.Int24;
                case 32:
                    return SampleType.Int32;
                default:
                    throw new NotSupportedException(String.Format("{0} bits per sample is not supported.", bitsPerSample));
            }
        }
    }
}
