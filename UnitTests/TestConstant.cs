using System;

namespace CrossTimeDsp.UnitTests
{
    internal class TestConstant
    {
        public const string ConfigurationFile = "UnitTestConfig.xml";
        public const string SourceFilePath16Bit = "whiteNoise.44100.16.wav";

        // definitions need to be in sync with the shifts defined in Constant.h
        public static readonly double Q15ToQ31TruncationErrorIncrease = Math.Pow(2.0, /* Constant.ShiftBetween16BitSamplesAndQ31 */ 14);
        public static readonly double Q23ToQ31TruncationErrorIncrease = Math.Pow(2.0, /* Constant.ShiftBetween24BitSamplesAndQ31 */ 6);
    }
}
