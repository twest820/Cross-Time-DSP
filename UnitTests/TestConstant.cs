using System;

namespace CrossTimeDsp.UnitTests
{
    internal class TestConstant
    {
        public const string ConfigurationFile = "UnitTestConfig.xml";
        public const string SourceFilePath16Bit = "whiteNoise.44100.16.wav";

        public static readonly double Q15ToQ31TruncationErrorIncrease = Math.Pow(2.0, Constant.Dsp.ShiftBetween16BitSamplesAndQ31);
        public static readonly double Q23ToQ31TruncationErrorIncrease = Math.Pow(2.0, Constant.Dsp.ShiftBetween24BitSamplesAndQ31);
    }
}
