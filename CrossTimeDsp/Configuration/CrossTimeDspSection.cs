using System;
using System.Configuration;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    [XmlRoot(ConfigurationConstants.CrossTimeDsp)]
    public class CrossTimeDspSection
    {
        [XmlElement(ConfigurationConstants.Engine)]
        public EngineElement Engine { get; set; }

        [XmlElement(ConfigurationConstants.Filters)]
        public FiltersElement Filters { get; set; }

        [XmlElement(ConfigurationConstants.Output)]
        public OutputElement Output { get; set; }

        public static CrossTimeDspSection Load()
        {
            // load the configuration section from app config
            CrossTimeDspSection configuration = (CrossTimeDspSection)ConfigurationManager.GetSection(ConfigurationConstants.CrossTimeDsp);
            if (configuration == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing {0} section.", ConfigurationConstants.CrossTimeDsp));
            }
            else if (configuration.Engine == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing {0}/{1} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Engine));
            }
            else if (configuration.Engine.Q31Adaptive == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing {0}/{1}/{2} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Engine, ConfigurationConstants.Q31Adaptive));
            }
            else if (configuration.Filters == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing {0}/{1} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Filters));
            }
            else if (configuration.Output == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing {0}/{1} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Output));
            }
            else if (configuration.Output.CollidingFileNamePostfix == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing the {0}/{1}/{2} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Output, ConfigurationConstants.CollidingFileNamePostfix));
            }
            else if (configuration.Output.Flac == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing {0}/{1}/{2} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Output, ConfigurationConstants.Flac));
            }
            else if (configuration.Output.Wav == null)
            {
                throw new ApplicationException(String.Format("Configuration file is missing {0}/{1}/{2} element.", ConfigurationConstants.CrossTimeDsp, ConfigurationConstants.Output, ConfigurationConstants.Flac));
            }
            // ConfigurationManager caches a singleton instance of the section.  So clone it so that changes to one CrossTimeEngine's settings
            // don't affect other instances of CrossTimeEngine
            return XmlConfigurationHandler<CrossTimeDspSection>.CloneSection(configuration);
        }
    }
}
