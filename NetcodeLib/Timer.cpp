#include "Timer.h"

Timer::Timer(uint64_t delayBetweenTicks) : DeadlineTimer{ IOEngine::GetIOContext() }, Duration{ boost::posix_time::milliseconds{delayBetweenTicks} }, Handlers{}, StopFlag{ false } {

}

Timer::~Timer() {
	Stop();
}

void Timer::InvokeFailHandlers(const std::string & msg) {
	for (auto i : Handlers) {
		i->TickFail(msg);
	}
}

void Timer::InvokeSuccessHandlers(double dt, double t) {
	for (auto i : Handlers) {
		i->TickSuccess(dt, t);
	}
}

void Timer::TimerLoop() {
	if(StopFlag) {
		return;
	}
	DeadlineTimer.expires_from_now(Duration);
	DeadlineTimer.async_wait([this](const boost::system::error_code & ec) -> void {
		if (ec != boost::system::errc::success) {
			if(!StopFlag) {
				InvokeFailHandlers(ec.message());
			}
		} else {
			CurrentTime = std::chrono::steady_clock::now();
			double dt = std::chrono::duration<double>((CurrentTime - LastTime)).count();
			double tt = std::chrono::duration<double>((CurrentTime - StartTime)).count();
			LastTime = CurrentTime;
			TimerLoop();
			InvokeSuccessHandlers(dt, tt);
		}
	});
}

void Timer::AddHandler(TickHandler & th) {
	Handlers.push_back(&th);
}

void Timer::Start() {
	StartTime = std::chrono::steady_clock::now();
	LastTime = std::chrono::steady_clock::now();
	TimerLoop();
}

void Timer::Stop() {
	StopFlag = true;
	DeadlineTimer.cancel();
}

void Timer::DelayedTimer::Stop() {
	DTimer->Stop();
	std::lock_guard<std::mutex> lock{ StaticMutex };
	Timers.erase(Id);
	delete this;
}
Timer::DelayedTimer::DelayedTimer(int id, std::function<void()> f, uint32_t msDelay) : Caller(f), DTimer(new Timer(msDelay)), Id{ id } {
	DTimer->AddHandler(Caller);
	DTimer->AddHandler(*this);
	DTimer->Start();
}

void Timer::DelayedTimer::TickSuccess(double dt, double t) {
	Stop();
}

void Timer::DelayedTimer::TickFail(const std::string & msg) {
	Stop();
}

Timer::DelayedTimer::~DelayedTimer() {
	delete DTimer;
}


std::mutex Timer::StaticMutex{};
 int Timer::NextTimerId = 0;
 std::map<int, Timer::DelayedTimer*> Timer::Timers{};

 void Timer::DelayCall(std::function<void()> f, uint32_t msDelay) {
	std::lock_guard<std::mutex> lock{ StaticMutex };
	if(NextTimerId == -1) {
		return;
	}
	int id = NextTimerId++;
	Timers[id] = new Timer::DelayedTimer{ id, f, msDelay };
}

 void Timer::CancelDelayedCalls() {
	 std::lock_guard<std::mutex> lock{ StaticMutex };
	 for(auto & i : Timers) {
		 i.second->Stop();
	 }
	 Timers.clear();
	 NextTimerId = -1;
 }

NamedTimers::NamedTimers() : Timers{} {

}

void NamedTimers::CreateTimer(const std::string & timerKey, uint64_t msBetweenTicks) {
	Timers.emplace(timerKey, msBetweenTicks);
}

Timer & NamedTimers::operator[](const std::string & key) {
	return Timers.find(key)->second;
}

void NamedTimers::Start() {
	for (auto & i : Timers) {
		i.second.Start();
	}
}

void NamedTimers::Stop() {
	for (auto & i : Timers) {
		i.second.Stop();
	}
}
