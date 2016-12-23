using CrossTimeDsp.Configuration;
using NAudio.MediaFoundation;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace CrossTimeDsp
{
    internal static class Constant
    {
        public const string ElapsedTimeFormat = @"mm\:ss\.fff";

        // must be a multiple of 12 bytes so an exact number of 16, 24, and 32 bit samples fit in the block, multiples of 24 preferred for stereo
        // must be a multiple of NativeDsp.Constant.FilterBlockSizeInBytes
        // Mesurements come in fastest with matching filter and sample block sizes around 36k
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
