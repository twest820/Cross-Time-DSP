using CrossTimeDsp.Dsp;
using System;
using System.Xml;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class ThirdOrderFilter : Filter
    {
        [XmlElement(Constant.Configuration.Biquad)]
        public Biquad Biquad { get; set; }
        [XmlElement(Constant.Configuration.FirstOrder)]
        public FirstOrderFilter FirstOrder { get; set; }

        public override void AddTo(FilterBank filters)
        {
            filters.AddThirdOrder(this.Biquad.Type, this.Biquad.F0, this.Biquad.Q, this.Biquad.GainInDB, this.FirstOrder.Type, this.FirstOrder.F0, this.FirstOrder.GainInDB);
        }

        public override void AdjustGain(double adjustmentInDB)
        {
            this.Biquad.AdjustGain(adjustmentInDB);
        }

        public override void Validate()
        {
            if (this.Biquad == null)
            {
                throw new XmlException(String.Format("Third order filter in {0}/{1} is missing a {2} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.Biquad));
            }
            if (this.FirstOrder == null)
            {
                throw new XmlException(String.Format("Third order filter in {0}/{1} is missing a {2} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.FirstOrder));
            }

            this.Biquad.TimeDirection = this.TimeDirection;
            this.FirstOrder.TimeDirection = this.TimeDirection;

            this.Biquad.Validate();
            this.FirstOrder.Validate();
        }
    }
}
