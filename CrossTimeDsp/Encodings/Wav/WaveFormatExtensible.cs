using System;
using System.Runtime.InteropServices;

namespace CrossTimeDsp.Encodings.Wav
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct WaveFormatExtensible
    {
        public WaveFormatEx Format;
        public UInt16 ValidBitsPerSample;
        public ChannelMask ChannelMask;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = WavConstants.GuidSize)]
        public byte[] SubFormat;

        public WaveFormatExtensible(int channels, int samplesPerSecond, int sampleSizeInBits, ChannelMask channelMask)
        {
            if (channels != 2)
            {
                throw new NotSupportedException("Currently only stereo data is supported.");
            }

            this.Format = new WaveFormatEx(channels, samplesPerSecond, sampleSizeInBits);
            this.Format.CommonFields.FormatTag = FormatTag.WAVE_FORMAT_EXTENSIBLE;
            // CBSize should always be 22
            this.Format.CBSize = WavConstants.WavFormatExtensibleSize - WavConstants.WavFormatExSize;
            this.ValidBitsPerSample = (UInt16)sampleSizeInBits;
            this.ChannelMask = channelMask;
            this.SubFormat = new Guid(WavConstants.KSDataFormatSubTypePcm).ToByteArray();
        }
    }
}
