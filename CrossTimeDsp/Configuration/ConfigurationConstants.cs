using System.Diagnostics.CodeAnalysis;

namespace CrossTimeDsp.Configuration
{
    internal static class ConfigurationConstants
    {
        public const string Biquad = "biquad";
        public const string BitsPerSample = "bitsPerSample";
        public const string CollidingFileNamePostfix = "collidingFileNamePostfix";
        public const string CompressionLevel = "compressionLevel";
        public const string CrossTimeDsp = "crossTimeDsp";
        public const string LamePath = "lamePath";
        public const string Encoding = "encoding";
        public const string Engine = "engine";
        public const string F0 = "f0";
        public const string Filters = "filters";
        public const string FirstOrder = "firstOrder";
        public const string Flac = "flac";
        public const string FlacPath = "flacPath";
        public const string GainInDB = "gainInDB";
        public const string MP3 = "mp3";
        public const string Output = "output";
        public const string Precision = "precision";
        public const string Q = "q";
        [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1310:FieldNamesMustNotContainUnderscore", Justification = "Casing.")]
        public const string Q31_32x64_Threshold = "q31_32x64_Threshold";
        [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1310:FieldNamesMustNotContainUnderscore", Justification = "Casing.")]
        public const string Q31_64x64_Threshold = "q31_64x64_Threshold";
        public const string Q31Adaptive = "q31Adaptive";
        public const string ReverseTimeAntiClippingAttenuationInDB = "reverseTimeAntiClippingAttenuationInDB";
        public const string TimeDirection = "timeDirection";
        public const string Type = "type";
        public const string Wav = "wav";
    }
}
