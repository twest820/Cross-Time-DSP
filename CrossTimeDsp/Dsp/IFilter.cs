namespace CrossTimeDsp.Dsp
{
    internal interface IFilter<TSample> where TSample : struct
    {
        IFilter<TSample> Clone();
        TSample Filter(TSample sample);
    }
}
