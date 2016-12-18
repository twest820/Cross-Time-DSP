namespace CrossTimeDsp.Dsp
{
    internal class GainQ31_64x64 : IFilter<int>
    {
        private Q63 scaleFactor;

        public GainQ31_64x64(double scaleFactor)
        {
            this.scaleFactor = new Q63(scaleFactor);
        }

        public void Filter(int[] block, int offset)
        {
            int maxSample = offset + Constant.FilterBlockSizeInInts;
            for (int sample = offset; sample < maxSample; ++sample)
            {
                block[sample] = (int)((this.scaleFactor * block[sample]) >> this.scaleFactor.FractionalBits);
            }
        }

        public void FilterReverse(int[] block, int offset)
        {
            this.Filter(block, offset);
        }
    }
}
