using System;
using System.Collections.Generic;
using System.IO;
using System.Xml;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    [XmlRoot(Constant.Configuration.CrossTimeDsp)]
    public class CrossTimeDspConfiguration
    {
        private static readonly XmlSerializer Serializer = new XmlSerializer(typeof(CrossTimeDspConfiguration));

        [XmlElement(Constant.Configuration.Engine)]
        public Engine Engine { get; set; }

        [XmlArray(Constant.Configuration.Filters)]
        [XmlArrayItem(Constant.Configuration.Biquad, Type = typeof(Biquad))]
        [XmlArrayItem(Constant.Configuration.FirstOrder, Type = typeof(FirstOrderFilter))]
        [XmlArrayItem(Constant.Configuration.LinearizeThreeWay, Type = typeof(LinearizeThreeWay))]
        [XmlArrayItem(Constant.Configuration.ThirdOrder, Type = typeof(ThirdOrderFilter))]
        public List<Filter> Filters { get; set; }

        [XmlIgnore]
        public DateTime LastWriteTimeUtc { get; set; }

        [XmlElement(Constant.Configuration.Output)]
        public Output Output { get; set; }

        public static CrossTimeDspConfiguration Load(string configurationFilePath)
        {
            // load the configuration section from app config
            CrossTimeDspConfiguration configuration;
            using (FileStream stream = new FileStream(configurationFilePath, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                configuration = (CrossTimeDspConfiguration)CrossTimeDspConfiguration.Serializer.Deserialize(stream);
            }
            configuration.LastWriteTimeUtc = File.GetLastWriteTimeUtc(configurationFilePath);

            if (configuration.Engine == null)
            {
                throw new XmlException(String.Format("Configuration file is missing the {0}/{1} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Engine));
            }
            if (configuration.Engine.Q31Adaptive == null)
            {
                throw new XmlException(String.Format("Configuration file is missing the {0}/{1}/{2} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Engine, Constant.Configuration.Q31Adaptive));
            }
            if (configuration.Engine.ReverseTimeAntiClippingAttenuationInDB >= 0.0)
            {
                throw new XmlException(String.Format("Configuration file is missing the {0}/{1}[@{2}] attribute or it is not set to an attenuation (a negative number of dB).", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Engine, Constant.Configuration.ReverseTimeAntiClippingAttenuationInDB));
            }

            if (configuration.Filters == null)
            {
                throw new XmlException(String.Format("Configuration file is missing the {0}/{1} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters));
            }
            if (configuration.Filters == null || configuration.Filters.Count < 1)
            {
                throw new XmlException(String.Format("Configuration file must define at least one filter in the {0}/{1} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Filters));
            }
            foreach (Filter filter in configuration.Filters)
            {
                filter.Validate();
            }

            if (configuration.Output == null)
            {
                throw new XmlException(String.Format("Configuration file is missing the {0}/{1} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Output));
            }
            if ((configuration.Output.BitsPerSample != 16) &&
                (configuration.Output.BitsPerSample != 24) &&
                (configuration.Output.BitsPerSample != 32))
            {
                throw new XmlException(String.Format("Configuration file is missing the {0}/{1}[@{2}] attribute or it is not set to 16, 24, or 32 bits.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Output, Constant.Configuration.BitsPerSample));
            }
            if (configuration.Output.CollidingFileNamePostfix == null)
            {
                // empty or whitespace is OK
                throw new XmlException(String.Format("Configuration file is missing the {0}/{1}/{2} element.", Constant.Configuration.CrossTimeDsp, Constant.Configuration.Output, Constant.Configuration.CollidingFileNamePostfix));
            }

            return configuration;
        }
    }
}
