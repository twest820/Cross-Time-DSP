namespace CrossTimeDsp.Dsp
{
    internal class GainDouble : IFilter<double>
    {
        private double gain;

        public GainDouble(double gain)
        {
            this.gain = gain;
        }

        public IFilter<double> Clone()
        {
            return new GainDouble(this.gain);
        }

        public double Filter(double sample)
        {
            return this.gain * sample;
        }
    }
}
