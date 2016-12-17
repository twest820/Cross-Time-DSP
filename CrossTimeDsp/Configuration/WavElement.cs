using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class WavElement
    {
        [XmlAttribute(ConfigurationConstants.BitsPerSample)]
        public int BitsPerSample { get; set; }
    }
}
