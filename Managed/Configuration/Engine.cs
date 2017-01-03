using CrossTimeDsp.Dsp;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class Engine
    {
        [XmlAttribute(Constant.Configuration.Precision)]
        public FilterPrecision Precision { get; set; }

        [XmlElement(Constant.Configuration.Q31Adaptive)]
        public Q31AdaptiveElement Q31Adaptive { get; set; }

        [XmlAttribute(Constant.Configuration.ReverseTimeAntiClippingAttenuationInDB)]
        public double ReverseTimeAntiClippingAttenuationInDB { get; set; }
    }
}
