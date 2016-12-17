using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class FlacElement
    {
        [XmlAttribute(ConfigurationConstants.BitsPerSample)]
        public int BitsPerSample { get; set; }

        [XmlAttribute(ConfigurationConstants.CompressionLevel)]
        public int CompressionLevel { get; set; }

        [XmlElement(ConfigurationConstants.FlacPath)]
        public string FlacPath { get; set; }
    }
}
