using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// Direct form 1 biquad implementation from the Bristow-Johnson cookbook at http://musicdsp.org/files/Audio-EQ-Cookbook.txt.
    /// </summary>
    /// <remarks>
    /// Stereo is special cased as it measures 53% faster (per perf test).
    /// </remarks>
    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "SA1650:ElementDocumentationMustBeSpelledCorrectly", Justification = "Url.")]
    internal class BiquadDouble : IFilter<double>
    {
        private double a1;
        private double a2;
        private double b0;
        private double b1;
        private double b2;
        private int channels;
        private double[] x1;
        private double[] x2;
        private double[] y1;
        private double[] y2;

        public BiquadDouble(double b0, double b1, double b2, double a0, double a1, double a2, int channels)
        {
            Debug.Assert(Constant.FilterBlockSizeInDoubles % 2 == 0, String.Format("Filter block size {0} does not support stereo optimizations.", Constant.FilterBlockSizeInDoubles));
            this.a1 = a1 / a0;
            this.a2 = a2 / a0;
            this.b0 = b0 / a0;
            this.b1 = b1 / a0;
            this.b2 = b2 / a0;

            this.channels = channels;
            this.x1 = new double[channels];
            this.x2 = new double[channels];
            this.y1 = new double[channels];
            this.y2 = new double[channels];
        }

        public void Filter(double[] block, int offset)
        {
            int maxSample = offset + Constant.FilterBlockSizeInDoubles;
            if (this.channels == 2)
            {
                Debug.Assert(offset % 2 == 0, String.Format("Offset {0} is not aligned to stereo block.", offset));
                for (int sample = offset; sample < maxSample; sample += 2)
                {
                    double value0 = block[sample];
                    double accumulator0 = this.b0 * value0 + this.b1 * this.x1[0] + this.b2 * this.x2[0] - this.a1 * this.y1[0] - this.a2 * this.y2[0];
                    this.x2[0] = this.x1[0];
                    this.x1[0] = value0;
                    this.y2[0] = this.y1[0];
                    this.y1[0] = accumulator0;
                    block[sample] = accumulator0;

                    double value1 = block[sample + 1];
                    double accumulator1 = this.b0 * value1 + this.b1 * this.x1[1] + this.b2 * this.x2[1] - this.a1 * this.y1[1] - this.a2 * this.y2[1];
                    this.x2[1] = this.x1[1];
                    this.x1[1] = value1;
                    this.y2[1] = this.y1[1];
                    this.y1[1] = accumulator1;
                    block[sample + 1] = accumulator1;
                }
            }
            else
            {
                for (int sample = offset; sample < maxSample; ++sample)
                {
                    int channel = sample % this.channels;
                    double value = block[sample];
                    double accumulator = this.b0 * value + this.b1 * this.x1[channel] + this.b2 * this.x2[channel] - this.a1 * this.y1[channel] - this.a2 * this.y2[channel];
                    this.x2[channel] = this.x1[channel];
                    this.x1[channel] = value;
                    this.y2[channel] = this.y1[channel];
                    this.y1[channel] = accumulator;
                    block[sample] = accumulator;
                }
            }
        }

        public void FilterReverse(double[] block, int offset)
        {
            int maxSample = offset + Constant.FilterBlockSizeInDoubles;
            if (this.channels == 2)
            {
                Debug.Assert(offset % 2 == 0, String.Format("Offset {0} is not aligned to stereo block.", offset));
                for (int sample = maxSample - 2; sample >= offset; sample -= 2)
                {
                    double value0 = block[sample];
                    double accumulator0 = this.b0 * value0 + this.b1 * this.x1[0] + this.b2 * this.x2[0] - this.a1 * this.y1[0] - this.a2 * this.y2[0];
                    this.x2[0] = this.x1[0];
                    this.x1[0] = value0;
                    this.y2[0] = this.y1[0];
                    this.y1[0] = accumulator0;
                    block[sample] = accumulator0;

                    double value1 = block[sample + 1];
                    double accumulator1 = this.b0 * value1 + this.b1 * this.x1[1] + this.b2 * this.x2[1] - this.a1 * this.y1[1] - this.a2 * this.y2[1];
                    this.x2[1] = this.x1[1];
                    this.x1[1] = value1;
                    this.y2[1] = this.y1[1];
                    this.y1[1] = accumulator1;
                    block[sample + 1] = accumulator1;
                }
            }
            else
            {
                for (int sample = maxSample - 1; sample >= offset; --sample)
                {
                    int channel = sample % this.channels;
                    double value = block[sample];
                    double accumulator = this.b0 * value + this.b1 * this.x1[channel] + this.b2 * this.x2[channel] - this.a1 * this.y1[channel] - this.a2 * this.y2[channel];
                    this.x2[channel] = this.x1[channel];
                    this.x1[channel] = value;
                    this.y2[channel] = this.y1[channel];
                    this.y1[channel] = accumulator;
                    block[sample] = accumulator;
                }
            }
        }
    }
}
