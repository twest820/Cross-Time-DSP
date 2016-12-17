using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class MP3Element
    {
        [XmlElement(ConfigurationConstants.LamePath)]
        public string LamePath { get; set; }
    }
}
