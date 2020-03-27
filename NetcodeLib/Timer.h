#pragma once

#include "IOEngine.h"
#include <boost/asio.hpp>
#include <chrono>
#include <mutex>

struct TickHandler {
	virtual void TickSuccess(double dt, double t) = 0;
	virtual void TickFail(const std::string & msg) = 0;
};

class DelayedFunctionCaller : public TickHandler {
	std::function<void()> Function;
public:

	DelayedFunctionCaller(std::function<void()> f) : Function(f) {

	}
	virtual void TickSuccess(double dt, double t) override {
		Function();
	}
	virtual void TickFail(const std::string & msg) override {
		//err
	}
};

class Timer {
	boost::asio::deadline_timer DeadlineTimer;
	boost::posix_time::time_duration Duration;
	std::vector<TickHandler*> Handlers;
	std::chrono::time_point<std::chrono::steady_clock> StartTime;
	std::chrono::time_point<std::chrono::steady_clock> LastTime;
	std::chrono::time_point<std::chrono::steady_clock> CurrentTime;
	bool StopFlag;

	void InvokeFailHandlers(const std::string & msg);
	void InvokeSuccessHandlers(double dt, double t);
	void TimerLoop();

	struct DelayedTimer : TickHandler {
		DelayedFunctionCaller Caller;
		Timer * DTimer;
		int Id;

		void Stop();
	public:
		DelayedTimer(int id, std::function<void()> f, uint32_t msDelay);
		virtual void TickSuccess(double dt, double t) override;
		virtual void TickFail(const std::string & msg) override;
		~DelayedTimer();
	};

	static std::mutex StaticMutex;
	static int NextTimerId;
	static std::map<int, DelayedTimer*> Timers;
public:
	Timer(uint64_t delayBetweenTicks);
	~Timer();

	void AddHandler(TickHandler & th);
	void Start();
	void Stop();

	static void DelayCall(std::function<void()> f, uint32_t msDelay);
	static void CancelDelayedCalls();
};

class NamedTimers {
protected:
	std::map<std::string, Timer> Timers;

public:
	NamedTimers();
	void CreateTimer(const std::string & timerKey, uint64_t msBetweenTicks);
	Timer & operator[](const std::string & key);
	void Start();
	void Stop();
};
