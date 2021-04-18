#ifndef Allocations_cpp
#define Allocations_cpp
#include <benchmark/benchmark.h>
#include "stacklin.cpp"
#include <vector>
#include <random>

std::size_t s_MaxAlignment = alignof(std::max_align_t);
std::size_t s_NumOfRandAllocations = 1e5;
std::size_t s_MaxBlockSize = 4096;
std::size_t s_1GB = 1024 * 1024 * 1024;
std::vector<std::size_t> s_Sizes;
std::vector<std::size_t> s_RandomSizes;
std::vector<std::size_t> s_DeallocationIndices;

namespace Utils
{
	int SelectRandomly(int min, int max)
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<std::mt19937::result_type> dis(min, max);
		return dis(gen);
	}
};


#endif



static void BenchmarkMultieRandomAllocationsAndFrees(Allocator* alloc, benchmark::State& state)
{
	auto it = s_DeallocationIndices.begin();
	std::vector<void*> addresses;

	for (auto _ : state)
	{
		for (std::size_t i = 0; i < state.range(0); ++i)
		{
			void* address;
			address = alloc->Allocate(s_RandomSizes[i], s_MaxAlignment);
			state.PauseTiming();
			if (it != s_DeallocationIndices.end() && *it == i)
			{
				addresses.push_back(address);
				++it;
			}
			state.ResumeTiming();
		}
		for (std::size_t i = 0; i < addresses.size(); ++i)
		{
			alloc->Deallocate(addresses[i]);
		}
		for (std::size_t i = 0; i < state.range(0); ++i)
		{
			benchmark::DoNotOptimize(alloc->Allocate(s_RandomSizes[i], s_MaxAlignment));
		}
		state.PauseTiming();
		alloc->Reset();
		addresses.clear();
		it = s_DeallocationIndices.begin();
		state.ResumeTiming();
	}
}

static void RandomAllocateAndFree_LinearAllocator(benchmark::State& state)
{
	LinearAllocator alloc(s_1GB);
	BenchmarkMultieRandomAllocationsAndFrees(&alloc, state);
	std::size_t randomSizesAgg = 0;
	for (unsigned int i = 0; i < state.range(0); ++i)
	{
		randomSizesAgg += s_RandomSizes[i];
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * randomSizesAgg);
}



static void RandomAllocateAndFree_StackAllocator(benchmark::State& state)
{
	StackAllocator alloc(s_1GB);
	BenchmarkMultieRandomAllocationsAndFrees(&alloc, state);
	std::size_t randomSizesAgg = 0;
	for (unsigned int i = 0; i < state.range(0); ++i)
	{
		randomSizesAgg += s_RandomSizes[i];
	}
	state.SetBytesProcessed(int64_t(state.iterations()) * randomSizesAgg);
}

