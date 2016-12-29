using System;

namespace CrossTimeDsp.UnitTests
{
    internal class TestConstant
    {
        public const string DefaultConfigurationFile = "BiquadFirstOrderDouble.xml";
        public const string ReferenceFilePath24Bit = "whiteNoise.InverseAllpass.44100.24.wav";
        public const string ReferenceFilePath16Bit = "whiteNoise.InverseAllpass.44100.16.wav";
        // scale factor is quirky as the initial 24 bit reference file was created with a slightly different approach towards
        // anti-clipping attenuation
        public const double ReferenceFileScaleFactor = 1.019154;
        public const int ShiftBetween16BitSamplesAndQ31 = 14;
        public const int ShiftBetween24BitSamplesAndQ31 = 6;

        public const string SourceFilePath16Bit = "whiteNoise.44100.16.wav";

        // definitions need to be in sync with the shifts defined in Constant.h
        public static readonly double Q15ToQ31TruncationErrorIncrease = Math.Pow(2.0, TestConstant.ShiftBetween16BitSamplesAndQ31);
        public static readonly double Q23ToQ31TruncationErrorIncrease = Math.Pow(2.0, TestConstant.ShiftBetween24BitSamplesAndQ31);
    }
}
