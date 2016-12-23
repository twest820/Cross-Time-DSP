using CrossTimeDsp.Dsp;
using System;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    /// <summary>
    /// Specification of a first order filter.
    /// </summary>
    public class FilterElement
    {
        [XmlAttribute(ConfigurationConstants.F0)]
        public double F0 { get; set; }

        [XmlAttribute(ConfigurationConstants.TimeDirection)]
        public TimeDirection TimeDirection { get; set; }

        [XmlAttribute(ConfigurationConstants.Type)]
        public FilterType Type { get; set; }
    }
}
