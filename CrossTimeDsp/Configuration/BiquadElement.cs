using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class BiquadElement : FilterElement
    {
        [XmlAttribute(ConfigurationConstants.GainInDB)]
        public double GainInDB { get; set; }

        [XmlAttribute(ConfigurationConstants.Q)]
        public double Q { get; set; }
    }
}
