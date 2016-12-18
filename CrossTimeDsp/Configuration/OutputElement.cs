using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class OutputElement
    {
        [XmlAttribute(ConfigurationConstants.BitsPerSample)]
        public int BitsPerSample { get; set; }

        [XmlElement(ConfigurationConstants.CollidingFileNamePostfix)]
        public string CollidingFileNamePostfix { get; set; }

        [XmlAttribute(ConfigurationConstants.Encoding)]
        public Encoding Encoding { get; set; }
    }
}
