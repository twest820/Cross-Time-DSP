using System.Collections.Generic;

namespace CrossTimeDsp.Dsp
{
    internal class FilterBank<TSample> where TSample : struct
    {
        private int channels;
        private int currentSample;
        private List<IFilter<TSample>> filters;

        public FilterBank(int channels)
        {
            this.channels = channels;
            this.currentSample = 0;
            this.filters = new List<IFilter<TSample>>();
        }

        public int FilterCount
        {
            get { return this.filters.Count; }
        }

        public void Add(IFilter<TSample> filter)
        {
            for (int channel = 0; channel < this.channels; ++channel)
            {
                this.filters.Add(filter.Clone());
            }
        }

        public TSample Filter(TSample sample)
        {
            TSample result = sample;
            for (int index = this.currentSample % this.channels; index < this.filters.Count; index += this.channels)
            {
                result = this.filters[index].Filter(result);
            }
            ++this.currentSample;
            return result;
        }

        public IEnumerable<TSample> Filter(IEnumerable<TSample> samples)
        {
            foreach (TSample sample in samples)
            {
                yield return this.Filter(sample);
            }
        }
    }
}
