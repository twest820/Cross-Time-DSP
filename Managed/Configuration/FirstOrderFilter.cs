using CrossTimeDsp.Dsp;
using System;
using System.Xml;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    /// <summary>
    /// Specification of a first order filter.
    /// </summary>
    public class FirstOrderFilter : Filter
    {
        [XmlAttribute(Constant.Configuration.F0)]
        public double F0 { get; set; }

        [XmlAttribute(Constant.Configuration.GainInDB)]
        public double GainInDB { get; set; }

        [XmlAttribute(Constant.Configuration.Type)]
        public FilterType Type { get; set; }

        public FirstOrderFilter()
        {
            this.GainInDB = 0.0;
            this.F0 = -1.0;
        }

        public override void AddTo(FilterBank filters)
        {
            filters.AddFirstOrder(this.Type, this.F0, this.GainInDB);
        }

        public override void AdjustGain(double adjustmentInDB)
        {
            this.GainInDB += adjustmentInDB;
        }

        public override void Validate()
        {
            if (this.F0 <= 0.0)
            {
                throw new XmlException(String.Format("Filter in {0}/{1} is missing an {2} attribute or its value isn't greater than zero.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.F0));
            }
        }
    }
}
