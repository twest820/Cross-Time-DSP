using CrossTimeDsp.Configuration;
using NAudio.MediaFoundation;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics.CodeAnalysis;

namespace CrossTimeDsp
{
    internal static class Constant
    {
        public const string ElapsedTimeFormat = @"mm\:ss\.fff";

        // must be a multiple of 12 bytes so an exact number of 16, 24, and 32 bit samples fit in the block, multiples of 24 preferred for stereo
        // Must be a multiple of Dsp::Constant::FilterBlockSizeInBytes.
        // Mesurements typically come in fastest with sample block sizes no larger than 36k.
        public const int SampleBlockSizeInBytes = 36 * 1024;

        public static readonly ReadOnlyDictionary<Encoding, Guid> EncodingGuids;
        public static readonly TimeSpan PowerShellMessagePollInterval;

        static Constant()
        {
            Constant.EncodingGuids = new ReadOnlyDictionary<Encoding, Guid>(new Dictionary<Encoding, Guid>()
            {
                { Encoding.AppleLossless, Guid.Parse("63616c61-0000-0010-8000-00aa00389b71") },
                { Encoding.AdvancedAudio, AudioSubtypes.MFAudioFormat_AAC },
                { Encoding.Flac, Guid.Parse("0000f1ac-0000-0010-8000-00aa00389b71") },
                { Encoding.Mp3, AudioSubtypes.MFAudioFormat_MP3 },
                { Encoding.Wave, AudioSubtypes.MFAudioFormat_PCM },
                { Encoding.WindowsMediaAudio9, AudioSubtypes.MFAudioFormat_WMAudioV9 },
                { Encoding.WindowsMediaAudioLossless, AudioSubtypes.MFAudioFormat_WMAudio_Lossless }
            });
            Constant.PowerShellMessagePollInterval = TimeSpan.FromMilliseconds(250);
        }

        public static class Configuration
        {
            public const string Biquad = "biquad";
            public const string BitsPerSample = "bitsPerSample";
            public const string CollidingFileNamePostfix = "collidingFileNamePostfix";
            public const string CrossTimeDsp = "crossTimeDsp";
            public const string Encoding = "encoding";
            public const string Engine = "engine";
            public const string F0 = "f0";
            public const string F1 = "f1";
            public const string Filters = "filters";
            public const string FirstOrder = "firstOrder";
            public const string GainInDB = "gainInDB";
            public const string HighCrossover = "highCrossover";
            public const string LinearizeThreeWay = "linearizeThreeWay";
            public const string LowCrossover = "lowCrossover";
            public const string MidRolloff = "midRolloff";
            public const string Output = "output";
            public const string Precision = "precision";
            public const string Q = "q";
            [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1310:FieldNamesMustNotContainUnderscore", Justification = "Casing.")]
            public const string Q31_32x64_Threshold = "q31_32x64_Threshold";
            [SuppressMessage("StyleCop.CSharp.NamingRules", "SA1310:FieldNamesMustNotContainUnderscore", Justification = "Casing.")]
            public const string Q31_64x64_Threshold = "q31_64x64_Threshold";
            public const string Q31Adaptive = "q31Adaptive";
            public const string ReverseTimeAntiClippingAttenuationInDB = "reverseTimeAntiClippingAttenuationInDB";
            public const string ThirdOrder = "thirdOrder";
            public const string TimeDirection = "timeDirection";
            public const string Type = "type";
            public const string Wav = "wav";
            public const string WooferRolloff = "wooferRolloff";
        }

        public static class Extension
        {
            public const string AppleLossless = ".m4a";
            public const string AdvancedAudio = ".mp4";
            public const string Flac = ".flac";
            public const string Mp3 = ".mp3";
            public const string Wave = ".wav";
            public const string WindowsAudio = ".wma";
        }

        public static class Int24
        {
            public static readonly int MaxValue;
            public static readonly byte[] MaxValueBytes;
            public static readonly int MinValue;
            public static readonly byte[] MinValueBytes;
            
            static Int24()
            {
                Int24.MaxValue = 8388607;
                Int24.MaxValueBytes = new byte[3] { 0xff, 0xff, 0x7f };
                Int24.MinValue = -8388608;
                Int24.MinValueBytes = new byte[3] { 0x00, 0x00, 0x80 };
            }
        }

        public static class MetadataTag
        {
            public const string Album = "ALBUM";
            public const string AlbumArtist = "ALBUMARTIST";
            public const string Artist = "ARTIST";
            public const string Composer = "COMPOSER";
            public const string Date = "DATE";
            public const string Genre = "GENRE";
            public const string Organization = "ORGANIZATION";
            public const string Title = "TITLE";
            public const string TrackNumber = "TRACKNUMBER";
            public const string Year = "YEAR";
        }
    }
}
