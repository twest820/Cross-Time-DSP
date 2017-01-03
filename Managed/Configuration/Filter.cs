using CrossTimeDsp.Dsp;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public abstract class Filter
    {
        [XmlAttribute(Constant.Configuration.TimeDirection)]
        public TimeDirection TimeDirection { get; set; }

        public abstract void AddTo(FilterBank filters);
        public abstract void AdjustGain(double adjustmentInDB);
        public abstract void Validate();
    }
}
