using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class OutputElement
    {
        [XmlElement(ConfigurationConstants.CollidingFileNamePostfix)]
        public string CollidingFileNamePostfix { get; set; }

        [XmlAttribute(ConfigurationConstants.Encoding)]
        public Encoding Encoding { get; set; }

        [XmlElement(ConfigurationConstants.Flac)]
        public FlacElement Flac { get; set; }

        [XmlElement(ConfigurationConstants.MP3)]
        public MP3Element MP3 { get; set; }

        [XmlElement(ConfigurationConstants.Wav)]
        public WavElement Wav { get; set; }
    }
}
