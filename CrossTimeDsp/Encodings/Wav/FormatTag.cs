namespace CrossTimeDsp.Encodings.Wav
{
    // if needed, add values from the 1994 wave update
    internal enum FormatTag : ushort
    {
        WAVE_FORMAT_PCM = 0x0001, // Microsoft Pulse Code Modulation (PCM) format
        WAVE_FORMAT_EXTENSIBLE = 0xfffe, // http://msdn.microsoft.com/en-us/windows/hardware/gg463006.aspx
    }
}
