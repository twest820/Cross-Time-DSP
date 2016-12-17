namespace CrossTimeDsp.Dsp
{
    public class DspConstants
    {
        /// <summary>
        /// Shift factor for converting between 16 bit audio samples and Q0.31 on [ 0.25, 0.25 ).  All 16 bits in the samples are 
        /// assumed to be used.
        /// </summary>
        public static readonly int ShiftBetween16BitSamplesAndQ31 = 14;

        /// <summary>
        /// Shift factor for converting between 24 bit audio samples and Q0.31 on [ 0.25, 0.25 ).  All 24 bits in the samples are 
        /// assumed to be used.
        /// </summary>
        public static readonly int ShiftBetween24BitSamplesAndQ31 = 6;
    }
}
