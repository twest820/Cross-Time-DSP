using CrossTimeDsp.Dsp;
using System;
using System.Xml;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class LinearizeThreeWay : Filter
    {
        [XmlAttribute(Constant.Configuration.LowCrossover)]
        public double LowCrossover { get; set; }

        [XmlAttribute(Constant.Configuration.GainInDB)]
        public double GainInDB { get; set; }

        [XmlAttribute(Constant.Configuration.HighCrossover)]
        public double HighCrossover { get; set; }

        [XmlAttribute(Constant.Configuration.MidRolloff)]
        public double MidRolloff { get; set; }

        [XmlAttribute(Constant.Configuration.WooferRolloff)]
        public double WooferRolloff { get; set; }

        public LinearizeThreeWay()
        {
            this.LowCrossover = -1.0;
            this.GainInDB = 0.0;
            this.HighCrossover = -1.0;
            this.MidRolloff = -1.0;
            this.WooferRolloff = -1.0;
            this.TimeDirection = TimeDirection.Reverse;
        }

        public override void AddTo(FilterBank filters)
        {
            filters.AddThreeWayLinearization(this.LowCrossover, this.HighCrossover, this.WooferRolloff, this.MidRolloff, this.GainInDB);
        }

        public override void AdjustGain(double adjustmentInDB)
        {
            this.GainInDB = 0.0;
        }

        public override void Validate()
        {
            if (this.WooferRolloff <= 0.0)
            {
                throw new XmlException(String.Format("Filter in {0}/{1} is missing a {2} attribute or its value isn't greater than zero.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.LowCrossover));
            }
            if (this.MidRolloff <= this.WooferRolloff)
            {
                throw new XmlException(String.Format("Filter in {0}/{1} is missing a {2} attribute or its value isn't greater than that of the {3} attribute.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.MidRolloff, Constant.Configuration.WooferRolloff));
            }
            if (this.LowCrossover <= this.WooferRolloff)
            {
                throw new XmlException(String.Format("Filter in {0}/{1} is missing a {2} attribute or its value isn't greater than that of the {3} attribute.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.LowCrossover, Constant.Configuration.WooferRolloff));
            }
            if (this.HighCrossover <= this.LowCrossover)
            {
                throw new XmlException(String.Format("Filter in {0}/{1} is missing a {2} attribute or its value isn't greater than that of the {3} attribute.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters, Constant.Configuration.HighCrossover, Constant.Configuration.LowCrossover));
            }
        }
    }
}
