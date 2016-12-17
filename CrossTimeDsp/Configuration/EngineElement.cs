using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class EngineElement
    {
        [XmlAttribute(ConfigurationConstants.Precision)]
        public FilterPrecision Precision { get; set; }

        [XmlElement(ConfigurationConstants.Q31Adaptive)]
        public Q31AdaptiveElement Q31Adaptive { get; set; }

        [XmlAttribute(ConfigurationConstants.ReverseTimeAntiClippingAttenuationInDB)]
        public double ReverseTimeAntiClippingAttenuationInDB { get; set; }
    }
}
