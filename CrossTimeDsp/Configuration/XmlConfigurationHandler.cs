using System;
using System.Configuration;
using System.IO;
using System.Xml;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class XmlConfigurationHandler<TSection> : IConfigurationSectionHandler
    {
        public static TSection CloneSection(TSection section)
        {
            MemoryStream buffer = new MemoryStream();
            XmlSerializer serializer = new XmlSerializer(typeof(TSection));
            serializer.Serialize(buffer, section);
            buffer.Seek(0, SeekOrigin.Begin);
            TSection clone = (TSection)serializer.Deserialize(buffer);
            return clone;
        }

        public object Create(object parent, object configContext, XmlNode section)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(TSection));
            return serializer.Deserialize(new XmlNodeReader(section));
        }
    }
}
