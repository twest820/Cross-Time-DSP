using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class Output
    {
        [XmlAttribute(Constant.Configuration.BitsPerSample)]
        public int BitsPerSample { get; set; }

        [XmlElement(Constant.Configuration.CollidingFileNamePostfix)]
        public string CollidingFileNamePostfix { get; set; }

        [XmlAttribute(Constant.Configuration.Encoding)]
        public Encoding Encoding { get; set; }
    }
}
