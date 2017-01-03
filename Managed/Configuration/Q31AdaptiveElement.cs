using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class Q31AdaptiveElement
    {
        [XmlAttribute(Constant.Configuration.Q31_32x64_Threshold)]
        public double Q31_32x64_Threshold { get; set; }
        [XmlAttribute(Constant.Configuration.Q31_64x64_Threshold)]
        public double Q31_64x64_Threshold { get; set; }
    }
}
