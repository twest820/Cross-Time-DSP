using System;

namespace CrossTimeDsp.Dsp
{
    internal class GainQ31 : IFilter<Int32>
    {
        private Q31 scaleFactor;

        public GainQ31(double scaleFactor)
        {
            this.scaleFactor = new Q31(scaleFactor);
        }

        private GainQ31(GainQ31 other)
        {
            this.scaleFactor = other.scaleFactor;
        }

        public IFilter<Int32> Clone()
        {
            return new GainQ31(this);
        }

        public Int32 Filter(Int32 sample)
        {
            return (Int32)((this.scaleFactor * sample) >> this.scaleFactor.FractionalBits);
        }
    }
}
