using System;
using System.Runtime.InteropServices;

namespace CrossTimeDsp.Encodings.Wav
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct WaveFormatEx
    {
        public WaveFormat CommonFields;
        public UInt16 CBSize;

        public WaveFormatEx(int channels, int samplesPerSecond, int sampleSizeInBits)
        {
            this.CommonFields = new WaveFormat(channels, samplesPerSecond, sampleSizeInBits);
            this.CBSize = 0;
        }
    }
}
