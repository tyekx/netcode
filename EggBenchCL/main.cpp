#include <benchmark/benchmark.h>
#include <future>
#include <thread>

static void BM_QueryFutureResult(benchmark::State& state) {

	std::promise<int> p;
	
	std::thread t([&p]() -> void {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		p.set_value(10);
	});
	std::future<int> f;
	if(f.valid()) {
		printf("valid 1\n");
	}

	f = p.get_future();

	if(f.valid()) {
		printf("valid 2\n");
	}

	for(auto _ : state) {
		f.wait_for(std::chrono::seconds(0));
	}

	t.join();


	if(f.valid()) {
		printf("valid 3\n");
	}

	f = p.get_future();

	if(f.valid()) {
		printf("valid %d\n", f.get());
	}

	printf("kek");
}

BENCHMARK(BM_QueryFutureResult);

BENCHMARK_MAIN();

