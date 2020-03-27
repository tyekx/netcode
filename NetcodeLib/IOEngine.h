#pragma once

#include <thread>
#include <vector>
#include <memory>
#include <boost/asio.hpp>

class IOEngine {
	static IOEngine * Instance;

	boost::asio::io_context ioContext;
	boost::asio::io_context::work work;
	std::vector<std::unique_ptr<std::thread>> Threads;

	IOEngine(int workers);
	~IOEngine();
public:

	static void Init(int workerThreadCount);
	static IOEngine * Get();
	static boost::asio::io_context & GetIOContext();
	static void Stop();
};
