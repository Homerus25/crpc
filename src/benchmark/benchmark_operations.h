#pragma once

#include <benchmark/benchmark.h>
#include <future>
#include <numeric>
#include <cmath>
#include <fstream>

#ifndef DATASIZE
#define DATASIZE 1024 * 64
#endif

static void FNBenchArguments(benchmark::internal::Benchmark* b) {
  b->UseRealTime()->Unit(benchmark::kMillisecond)->Arg(1)->Arg(2)->Arg(4)->Arg(8)->Arg(16)->Arg(32)->Arg(64)->Arg(128)->Arg(256);//->Arg(64);
}

static void set_stats(benchmark::State& state, const int concurrency,
                      const int requests) {
  state.counters["Requests"] = concurrency * requests;
  state.counters["Req_per_second"] = benchmark::Counter(
      concurrency * requests, benchmark::Counter::kIsRate);
}

static void concurrent_bench(benchmark::State& state, const int concurrency,
                             const int requests,
                             auto fn) {
  std::vector<std::future<void>> clientRes;
  clientRes.reserve(concurrency);

  for(auto _ : state) {
    for (int i = 0; i < concurrency; ++i) {
      clientRes.emplace_back(std::async(std::launch::async, [&]() {
        for (int n = 0; n < requests; ++n) {
          fn();
        }
      }));
    }

    for (auto& clf : clientRes) {
      clf.wait();
    }
  }
}

static void BM_FN0(benchmark::State& state) {
  const int concurrency = state.range(0);
  const int requests = 100000;

  set_stats(state, concurrency, requests);

  auto fn = [](std::string ms) {
    return std::string("hello " + ms);
  };

  concurrent_bench(state, concurrency, requests, [&]() { benchmark::DoNotOptimize(fn("peter")); });
}

BENCHMARK(BM_FN0)->Apply(FNBenchArguments);//->Unit(benchmark::kMillisecond);;

static void BM_FN1(benchmark::State& state) {
  const int concurrency = state.range(0);
  const int requests = 10000;

  set_stats(state, concurrency, requests);

  auto fn = [](const std::vector<double>& vec) {
    double avg = 0.;
    for (double element : vec) avg += element / vec.size();
    return avg;
  };

  concurrent_bench(state, concurrency, requests, [&]() { benchmark::DoNotOptimize(fn(std::vector<double>(DATASIZE / sizeof(double)))); });
}

BENCHMARK(BM_FN1)->Apply(FNBenchArguments);//->Unit(benchmark::kMillisecond);;

static void BM_FN2(benchmark::State& state) {
  const int concurrency = state.range(0);
  const int requests = 10000;

  set_stats(state, concurrency, requests);

  auto fn = [](int size) {
    std::vector<double> ret(size);
    std::iota(ret.begin(), ret.end(), 0.);
    return ret;
  };

  concurrent_bench(state, concurrency, requests, [&]() { benchmark::DoNotOptimize(fn(int(DATASIZE / sizeof(double)))); });
}

BENCHMARK(BM_FN2)->Apply(FNBenchArguments);//->Unit(benchmark::kMillisecond);;

static void BM_FN3(benchmark::State& state) {
  const int concurrency = state.range(0);
  const int requests = 10000;

  set_stats(state, concurrency, requests);

  auto fn = [](std::vector<unsigned char> l_data) { return l_data; };

  concurrent_bench(state, concurrency, requests, [&]() { benchmark::DoNotOptimize(fn(std::vector<unsigned char>(DATASIZE / sizeof(unsigned char)))); });
}

BENCHMARK(BM_FN3)->Apply(FNBenchArguments);//->Unit(benchmark::kMillisecond);;







static void FNLatencyBenchArguments(benchmark::internal::Benchmark* b) {
  b->UseRealTime()->Iterations(1)->Unit(benchmark::kMillisecond);
}

static void latency_bench(benchmark::State& state, std::string filename,
                          const int requests,
                          auto fn) {
  std::vector<std::chrono::nanoseconds> times;
  times.reserve(requests);

  for(auto _ : state) {
    for (int n = 0; n < requests; ++n) {
      auto start = std::chrono::high_resolution_clock::now();
      fn();
      auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
      times.push_back(dur);
    }
  }

  std::sort(times.begin(), times.end());
  std::fstream file(filename, std::ios_base::out);
  file << times[std::round(requests * 0.05)] << ", " << times[std::round(requests * 0.5)] << ", "
       << times[std::round(requests * 0.95)] << ", " << times[std::round(requests * 0.99)];
}

static void BM_Latency_FN0(benchmark::State& state) {
  const int requests = 1000000;
  auto fn = [](std::string ms) {
    return std::string("hello " + ms);
  };
  latency_bench(state, "say-hello.txt", requests, [&]() { benchmark::DoNotOptimize(fn("peter")); });
}

static void BM_Latency_FN1(benchmark::State& state) {
  const int requests = 1000000;
  auto fn = [](const std::vector<double>& vec) {
    double avg = 0.;
    for (double element : vec) avg += element / vec.size();
    return avg;
  };
  latency_bench(state, "average.txt", requests, [&]() { benchmark::DoNotOptimize(fn(std::vector<double>(DATASIZE / sizeof(double)))); });
}

static void BM_Latency_FN2(benchmark::State& state) {
  const int requests = 1000000;
  auto fn = [](int size) {
    std::vector<double> ret(size);
    std::iota(ret.begin(), ret.end(), 0.);
    return ret;
  };
  latency_bench(state, "get-numbers.txt", requests, [&]() { benchmark::DoNotOptimize(fn(int(DATASIZE / sizeof(double)))); });
}

static void BM_Latency_FN3(benchmark::State& state) {
  const int requests = 1000000;
  auto fn = [](std::vector<unsigned char> l_data) { return l_data; };
  latency_bench(state, "large-data.txt", requests, [&]() { benchmark::DoNotOptimize(fn(std::vector<unsigned char>(DATASIZE / sizeof(unsigned char)))); });
}

BENCHMARK(BM_Latency_FN0)->Apply(FNLatencyBenchArguments);
BENCHMARK(BM_Latency_FN1)->Apply(FNLatencyBenchArguments);
BENCHMARK(BM_Latency_FN2)->Apply(FNLatencyBenchArguments);
BENCHMARK(BM_Latency_FN3)->Apply(FNLatencyBenchArguments);