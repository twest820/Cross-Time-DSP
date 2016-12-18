using System;
using System.Collections.Generic;

namespace CrossTimeDsp.Dsp
{
    internal class FilterBank
    {
        private List<IFilter<double>> doubleFilters;
        private List<IFilter<int>> intFilters;

        public FilterBank()
        {
            this.doubleFilters = new List<IFilter<double>>();
            this.intFilters = new List<IFilter<int>>();
        }

        public int FilterCount
        {
            get { return this.doubleFilters.Count + this.intFilters.Count; }
        }

        public void Add(IFilter<double> filter)
        {
            this.doubleFilters.Add(filter);
        }

        public void Add(IFilter<int> filter)
        {
            this.intFilters.Add(filter);
        }

        public SampleBlock Filter(SampleBlock block, SampleType dataPathSampleType, SampleType outputSampleType)
        {
            block = block.ConvertTo(dataPathSampleType);
            switch (dataPathSampleType)
            {
                case SampleType.Double:
                    this.Filter<double>(this.doubleFilters, block.DoubleBuffer, block.DoubleBufferCount, Constant.FilterBlockSizeInDoubles);
                    break;
                case SampleType.Int32:
                    this.Filter<int>(this.intFilters, block.IntBuffer, block.IntBufferCount, Constant.FilterBlockSizeInInts);
                    break;
                default:
                    throw new NotSupportedException(String.Format("Unhandled data path sample type {0}."));
            }

            return block.ConvertTo(outputSampleType);
        }

        private void Filter<TSample>(List<IFilter<TSample>> filters, TSample[] block, int blockLength, int filterBlockSizeInSamples) where TSample : struct
        {
            for (int index = 0; index < blockLength; index += filterBlockSizeInSamples)
            {
                for (int filter = 0; filter < filters.Count; ++filter)
                {
                    filters[filter].Filter(block, index);
                }
            }
        }

        public SampleBlock FilterReverse(SampleBlock block, SampleType dataPathSampleType, SampleType outputSampleType)
        {
            block = block.ConvertTo(dataPathSampleType);
            switch (dataPathSampleType)
            {
                case SampleType.Double:
                    int count = block.DoubleBufferCount;
                    if (count % Constant.FilterBlockSizeInDoubles != 0)
                    {
                        count = (count / Constant.FilterBlockSizeInDoubles + 1) * Constant.FilterBlockSizeInDoubles;
                    }
                    this.FilterReverse<double>(this.doubleFilters, block.DoubleBuffer, count, Constant.FilterBlockSizeInDoubles);
                    break;
                case SampleType.Int32:
                    count = block.IntBufferCount;
                    if (count % Constant.FilterBlockSizeInInts != 0)
                    {
                        count = (count / Constant.FilterBlockSizeInInts + 1) * Constant.FilterBlockSizeInInts;
                    }
                    this.FilterReverse<int>(this.intFilters, block.IntBuffer, count, Constant.FilterBlockSizeInInts);
                    break;
                default:
                    throw new NotSupportedException(String.Format("Unhandled data path sample type {0}."));
            }

            return block.ConvertTo(outputSampleType);
        }

        private void FilterReverse<TSample>(List<IFilter<TSample>> filters, TSample[] block, int count, int filterBlockSizeInSamples) where TSample : struct
        {
            for (int index = count - filterBlockSizeInSamples; index >= 0; index -= filterBlockSizeInSamples)
            {
                for (int filter = 0; filter < filters.Count; ++filter)
                {
                    filters[filter].FilterReverse(block, index);
                }
            }
        }
    }
}
