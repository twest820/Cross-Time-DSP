namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// direct form 1 implementation of a first order filter
    /// </summary>
    internal class FirstOrderFilterDouble : IFilter<double>
    {
        private double a1;
        private double b0;
        private double b1;
        private double x1;
        private double y1;

        public FirstOrderFilterDouble(double b0, double b1, double a0, double a1)
        {
            this.a1 = a1 / a0;
            this.b0 = b0 / a0;
            this.b1 = b1 / a0;
            this.x1 = 0.0;
            this.y1 = 0.0;
        }

        public IFilter<double> Clone()
        {
            return new FirstOrderFilterDouble(this.b0, this.b1, 1.0, this.a1);
        }

        public double Filter(double sample)
        {
            double result = this.b0 * sample + this.b1 * this.x1 - this.a1 * this.y1;
            this.x1 = sample;
            this.y1 = result;
            return result;
        }
    }
}