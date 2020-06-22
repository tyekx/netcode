#include "Modules.h"
#include <thread>
#include <atomic>

namespace Netcode::Module {

	class ConsoleModule : public IWindowModule {
		std::atomic_bool keepRunning;
		std::vector<std::string> commands;
		std::unique_ptr<std::thread> thread;
		std::mutex mutex;
	public:
		virtual void Start(AApp * app, Netcode::Config * config) override;

		virtual void Shutdown() override;

		virtual void * GetUnderlyingPointer() override;

		virtual void ProcessMessages() override;

		virtual void CompleteFrame() override;

		virtual void ShowWindow() override;

		virtual bool KeepRunning() override;

		virtual void ShowDebugWindow() override;
	};

}

