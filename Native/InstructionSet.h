// adapted from https://msdn.microsoft.com/en-us/library/hskdteyh.aspx
#pragma once
#include <bitset>

namespace CrossTimeDsp::Dsp
{
	class InstructionSet
	{
	private:
		class CpuInfo
		{
		public:
			CpuInfo();

			std::bitset<32> function1_Ecx;
			std::bitset<32> function7_Ebx;
		};
		
		static const CpuInfo Cpu;

	public:
		__declspec(dllexport) static bool Avx();
		__declspec(dllexport) static bool Avx2();
		__declspec(dllexport) static bool Fma();
	};
}