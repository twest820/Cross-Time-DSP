using System;
using System.IO;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    [XmlRoot(ConfigurationConstants.CrossTimeDsp)]
    public class CrossTimeDspConfiguration
    {
        private static readonly XmlSerializer Serializer = new XmlSerializer(typeof(CrossTimeDspConfiguration));

        [XmlElement(ConfigurationConstants.Engine)]
        public EngineElement Engine { get; set; }

        [XmlElement(ConfigurationConstants.Filters)]
        public FiltersElement Filters { get; set; }

        [XmlIgnore]
        public DateTime LastWriteTimeUtc { get; set; }

        [XmlElement(ConfigurationConstants.Output)]
        public OutputElement Output { get; set; }

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
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Engine));
            }
            if (configuration.Engine.Q31Adaptive == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1}/{2} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Engine, ConfigurationConstants.Q31Adaptive));
            }
            if (configuration.Engine.ReverseTimeAntiClippingAttenuationInDB >= 0.0)
            {
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1}[@{2}] attribute or it is not set to an attenuation (a negative number of dB).", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Engine, ConfigurationConstants.ReverseTimeAntiClippingAttenuationInDB));
            }

            if (configuration.Filters == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Filters));
            }
            if (configuration.Filters.Filters == null || configuration.Filters.Filters.Count < 1)
            {
                throw new ApplicationException(String.Format("Configuration file must define at least one filter in the {0}/{1} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Filters));
            }
            foreach (FilterElement filter in configuration.Filters.Filters)
            {
                if (filter.F0 <= 0.0)
                {
                    throw new ApplicationException(String.Format("Filter in {0}/{1} is missing an {2} attribute or its value isn't greater than zero.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Filters, ConfigurationConstants.F0));
                }
            }

            if (configuration.Output == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Output));
            }
            if ((configuration.Output.BitsPerSample != 16) &&
                (configuration.Output.BitsPerSample != 24) &&
                (configuration.Output.BitsPerSample != 32))
            {
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1}[@{2}] attribute or it is not set to 16, 24, or 32 bits.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Output, ConfigurationConstants.BitsPerSample));
            }
            if (configuration.Output.CollidingFileNamePostfix == null)
            {
                // empty or whitespace is OK
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1}/{2} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Output, ConfigurationConstants.CollidingFileNamePostfix));
            }

            return configuration;
        }
    }
}
