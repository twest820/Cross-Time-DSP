### Overview
This repository contains the source code for and releases of Cross Time DSP.  See the wiki for documentation.

### Contributing
Bug reports, feature requests, and feedback are most welcome.  Pull requests just solving whatever it is are even better.

### Development Environment
Install [Visual Studio 2015 Community](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx) Update 3 or newer with the below options in addition to the defaults:

* Common Tools -> GitHub Extension for Visual Studio

Higher Visual Studio SKUs such as Enterprise are fine.  After Visual Studio installation:

* clone the repo locally through Visual Studio's Team Explorer or GitHub's clone or download options
* install Visual StyleCop (Tools -> Extensions and Updates -> Online)

Commits should

* include appropriate test coverage
* have no build warnings, pass code analysis (Analyze -> Run Code Analysis), and be free of StyleCop issues (right click solution -> Run StyleCop)

Application and test development is done against .NET 4.6.

Also helpful is Atlassian's [https://www.atlassian.com/software/sourcetree](SourceTree), an augmentation of the git support available in Visual Studio's Team Explorer.

### Dependencies
* Cross Time DSP is supported and tested on Windows 10 Anniversary Update.  Windows 10 is required for ALAC or FLAC processing.
* Cross Time DSP should also run without issue on Windows Server 2008 or newer and legacy Windows 8.1, 8, and 7 SP1 systems not updated to Windows 10.  Support is, however, limited.
* Windows Vista SP2 and earlier and 32 bit versions of Windows are not supported.  Windows 7 users will need to [install .NET 4.6 or newer](https://msdn.microsoft.com/en-us/library/bb822049.aspx) if it's not already present.
* Cross Time DSP requires an [Intel Core i3, i5, or i7 processor](https://en.wikipedia.org/wiki/Intel_Core) or equivalent.  45nm Core 2 processors (Penryn) will also work.
* Visual Studio 2015 Community Update 3 or higher is required for development.

