using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class Q31AdaptiveElement
    {
        [XmlAttribute(ConfigurationConstants.Q31_32x64_Threshold)]
        public double Q31_32x64_Threshold { get; set; }
        [XmlAttribute(ConfigurationConstants.Q31_64x64_Threshold)]
        public double Q31_64x64_Threshold { get; set; }
    }
}
