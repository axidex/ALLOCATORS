#include <benchmark/benchmark.h>
#include "Allocations.cpp"

static void SetUp()
{
	s_Sizes = { 1, 2, 4, 8, 16, 32, 64, 256, 512, 1024, 2048, 4096, 8192 };
	s_RandomSizes.reserve(s_NumOfRandAllocations);
	for (std::size_t i = 0; i < s_RandomSizes.capacity(); ++i)
	{
		std::size_t size = (std::size_t)Utils::SelectRandomly(1, (int)s_MaxBlockSize);
		s_RandomSizes.push_back(size);
		if (Utils::SelectRandomly(1, 4) == 1)
			s_DeallocationIndices.push_back(i);
	}
}

BENCHMARK(RandomAllocateAndFree_LinearAllocator)->DenseRange(0, s_NumOfRandAllocations, 10000)->Unit(benchmark::kMillisecond);
BENCHMARK(RandomAllocateAndFree_StackAllocator)->DenseRange(0, s_NumOfRandAllocations, 10000)->Unit(benchmark::kMillisecond);

int main(int argc, char** argv)
{
	SetUp();
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
}
