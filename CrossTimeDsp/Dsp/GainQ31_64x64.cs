using System;

namespace CrossTimeDsp.Dsp
{
    internal class GainQ31_64x64 : IFilter<Int32>
    {
        private Q63 scaleFactor;

        public GainQ31_64x64(double scaleFactor)
        {
            this.scaleFactor = new Q63(scaleFactor);
        }

        private GainQ31_64x64(GainQ31_64x64 other)
        {
            this.scaleFactor = other.scaleFactor;
        }

        public IFilter<Int32> Clone()
        {
            return new GainQ31_64x64(this);
        }

        public Int32 Filter(Int32 sample)
        {
            return (Int32)((this.scaleFactor * sample) >> this.scaleFactor.FractionalBits);
        }
    }
}
