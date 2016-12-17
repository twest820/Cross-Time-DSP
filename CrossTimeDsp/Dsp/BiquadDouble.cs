using System.Diagnostics.CodeAnalysis;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// direct form 1 biquad implementation from the Bristow-Johnson cookbook at http://musicdsp.org/files/Audio-EQ-Cookbook.txt
    /// </summary>
    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "SA1650:ElementDocumentationMustBeSpelledCorrectly", Justification = "Url.")]
    internal class BiquadDouble : IFilter<double>
    {
        private double a1;
        private double a2;
        private double b0;
        private double b1;
        private double b2;
        private double x1;
        private double x2;
        private double y1;
        private double y2;

        public BiquadDouble(double b0, double b1, double b2, double a0, double a1, double a2)
        {
            this.a1 = a1 / a0;
            this.a2 = a2 / a0;
            this.b0 = b0 / a0;
            this.b1 = b1 / a0;
            this.b2 = b2 / a0;
            this.x1 = 0.0;
            this.x2 = 0.0;
            this.y1 = 0.0;
            this.y2 = 0.0;
        }

        public IFilter<double> Clone()
        {
            return new BiquadDouble(this.b0, this.b1, this.b2, 1.0, this.a1, this.a2);
        }

        public double Filter(double sample)
        {
            double result = this.b0 * sample + this.b1 * this.x1 + this.b2 * this.x2 - this.a1 * this.y1 - this.a2 * this.y2;
            this.x2 = this.x1;
            this.x1 = sample;
            this.y2 = this.y1;
            this.y1 = result;
            return result;
        }
    }
}
