using System.Collections.Generic;
using System.Xml.Serialization;

namespace CrossTimeDsp.Configuration
{
    public class FiltersElement
    {
        [XmlElement(ConfigurationConstants.Biquad, Type = typeof(BiquadElement))]
        [XmlElement(ConfigurationConstants.FirstOrder, Type = typeof(FilterElement))]
        public List<FilterElement> Filters { get; set; }
    }
}
