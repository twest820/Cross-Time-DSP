// adapted from https://msdn.microsoft.com/en-us/library/hskdteyh.aspx
#pragma once
#include <bitset>

namespace CrossTimeDsp::Dsp
{
	public class InstructionSet
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
		static bool Avx() { return Cpu.function1_Ecx[28]; }
		static bool Avx2() { return Cpu.function7_Ebx[5]; }
		static bool Fma() { return Cpu.function1_Ecx[12]; }
		static bool Sse41() { return Cpu.function1_Ecx[19]; }
	};
}