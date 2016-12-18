namespace CrossTimeDsp.Dsp
{
    internal class GainDouble : IFilter<double>
    {
        private double gain;

        public GainDouble(double gain)
        {
            this.gain = gain;
        }

        public void Filter(double[] block, int offset)
        {
            int maxSample = offset + Constant.FilterBlockSizeInDoubles;
            for (int sample = offset; sample < maxSample; ++sample)
            {
                block[sample] *= this.gain;
            }
        }

        public void FilterReverse(double[] block, int offset)
        {
            this.Filter(block, offset);
        }
    }
}
