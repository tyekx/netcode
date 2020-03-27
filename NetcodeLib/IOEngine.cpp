#include "IOEngine.h"

IOEngine * IOEngine::Instance = nullptr;

IOEngine::IOEngine(int workers) : ioContext{}, work{ ioContext }, Threads{} {
	for (int i = 0; i < workers; ++i) {
		Threads.emplace_back(new std::thread{ [this]() -> void {
			this->ioContext.run();
		} });
	}
}

IOEngine::~IOEngine() {
	ioContext.stop();
	for (auto & i : Threads) {
		i->join();
	}
}

void IOEngine::Init(int workerThreadCount) {
	workerThreadCount = (workerThreadCount < 1) ? 1 : workerThreadCount;
	workerThreadCount = (workerThreadCount > 200) ? 200 : workerThreadCount;
	if (Instance == nullptr) {
		Instance = new IOEngine(workerThreadCount);
	}
}

IOEngine * IOEngine::Get() {
	return Instance;
}

boost::asio::io_service & IOEngine::GetIOContext() {
	return Get()->ioContext;
}

void IOEngine::Stop() {
	delete Instance;
	Instance = nullptr;
}
