#include "stdafx.h"
#include <iostream>
#include "Avx.h"
#include "Constant.h"

using namespace std;
using namespace CrossTimeDsp::Dsp;
using namespace CrossTimeDsp::UnitTestShellNative;

int main()
{
	__int32 order = 6;
	Avx avx(2 * 44100 * (60 * 3 + 7), 3 * Constant::FilterBlockSizeInBytes);

	cout << "warmup" << endl;
	for (int warmupIteration = 0; warmupIteration < 10; ++warmupIteration)
	{
		avx.FilterBlocks(order, true);
	}

	cout << "running" << endl;
	for (int iteration = 0; iteration < 30; ++iteration)
	{
		avx.FilterBlocks(order, false);
	}
	return 0;
}