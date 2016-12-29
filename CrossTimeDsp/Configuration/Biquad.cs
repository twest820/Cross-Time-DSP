using CrossTimeDsp.Dsp;
using System;
using System.Xml;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class Biquad : FirstOrderFilter
    {
        [XmlAttribute(Constant.Configuration.Q)]
        public double Q { get; set; }

        public Biquad()
        {
            this.Q = -1.0;
        }

        public override void AddTo(FilterBank filters)
        {
            filters.AddBiquad(this.Type, this.F0, this.Q, this.GainInDB);
        }

        public override void Validate()
        {
            base.Validate();

            if (this.Q <= 0.0)
            {
                throw new XmlException(String.Format("Filter in {0}/{1} is missing an {2} attribute or its value isn't greater than zero.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.Q));
            }
        }
    }
}
