using System;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace CrossTimeDsp.Encodings.Wav
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct WaveFormat
    {
        public FormatTag FormatTag;
        public UInt16 Channels;
        public UInt32 SamplesPerSecond;
        public UInt32 AverageBytesPerSec;
        public UInt16 BlockAlign;
        public UInt16 BitsPerSample;

        [SuppressMessage("StyleCop.CSharp.ReadabilityRules", "SA1101:PrefixLocalCallsWithThis", Justification = "StyleCop bug.")]
        public WaveFormat(int channels, int samplesPerSecond, int sampleSizeInBits)
        {
            this.AverageBytesPerSec = (UInt32)(channels * samplesPerSecond * sampleSizeInBits / 8);
            this.BitsPerSample = (UInt16)sampleSizeInBits;
            this.BlockAlign = (UInt16)(channels * sampleSizeInBits / 8);
            this.Channels = (UInt16)channels;
            this.FormatTag = FormatTag.WAVE_FORMAT_PCM;
            this.SamplesPerSecond = (UInt32)samplesPerSecond;
        }
    }
}
