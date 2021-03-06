#include "stdafx.h"
#include <array>  
#include <bitset>  
#include <vector>  
#include "InstructionSet.h"

namespace CrossTimeDsp::Dsp
{
	const InstructionSet::CpuInfo InstructionSet::Cpu;

	#pragma unmanaged
	InstructionSet::CpuInfo::CpuInfo() :
		function1_Ecx{ 0 },
		function7_Ebx{ 0 }
	{
		// calling __cpuid with 0x0 as the function_id argument gets the number of the highest valid function ID
		std::array<int, 4> cpuIdentifiers;
		__cpuid(cpuIdentifiers.data(), 0);
		int identifiers = cpuIdentifiers[0];

		std::vector<std::array<int, 4>> cpuIdentifiersExtended;
		std::vector<std::array<int, 4>> extdata_;
		for (int identifier = 0; identifier <= identifiers; ++identifier)
		{
			__cpuidex(cpuIdentifiers.data(), identifier, 0);
			cpuIdentifiersExtended.push_back(cpuIdentifiers);
		}

		// load bitset with flags for function 0x00000001  
		if (identifiers >= 1)
		{
			this->function1_Ecx = cpuIdentifiersExtended[1][2];
		}

		// load bitset with flags for function 0x00000007  
		if (identifiers >= 7)
		{
			this->function7_Ebx = cpuIdentifiersExtended[7][1];
		}
	}
}