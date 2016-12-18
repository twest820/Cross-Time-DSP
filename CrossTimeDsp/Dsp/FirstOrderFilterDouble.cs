using System;
using System.Diagnostics;

namespace CrossTimeDsp.Dsp
{
    /// <summary>
    /// Direct form 1 implementation of a first order filter.
    /// </summary>
    internal class FirstOrderFilterDouble : IFilter<double>
    {
        private double a1;
        private double b0;
        private double b1;
        private int channels;
        private double[] x1;
        private double[] y1;

        public FirstOrderFilterDouble(double b0, double b1, double a0, double a1, int channels)
        {
            this.a1 = a1 / a0;
            this.b0 = b0 / a0;
            this.b1 = b1 / a0;

            this.channels = channels;
            this.x1 = new double[channels];
            this.y1 = new double[channels];
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
                    double accumulator0 = this.b0 * value0 + this.b1 * this.x1[0] - this.a1 * this.y1[0];
                    this.x1[0] = value0;
                    this.y1[0] = accumulator0;
                    block[sample] = accumulator0;

                    double value1 = block[sample + 1];
                    double accumulator1 = this.b0 * value1 + this.b1 * this.x1[1] - this.a1 * this.y1[1];
                    this.x1[1] = value1;
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
                    double accumulator = this.b0 * value + this.b1 * this.x1[channel] - this.a1 * this.y1[channel];
                    this.x1[channel] = value;
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
                    double accumulator0 = this.b0 * value0 + this.b1 * this.x1[0] - this.a1 * this.y1[0];
                    this.x1[0] = value0;
                    this.y1[0] = accumulator0;
                    block[sample] = accumulator0;

                    double value1 = block[sample + 1];
                    double accumulator1 = this.b0 * value1 + this.b1 * this.x1[1] - this.a1 * this.y1[1];
                    this.x1[1] = value1;
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
                    double accumulator = this.b0 * value + this.b1 * this.x1[channel] - this.a1 * this.y1[channel];
                    this.x1[channel] = value;
                    this.y1[channel] = accumulator;
                    block[sample] = accumulator;
                }
            }
        }
    }
}