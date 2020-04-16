#include "Modules.h"

namespace Netcode::Module {

	class ConsoleModule : public IWindowModule {
		bool keepRunning;
	public:
		virtual void Start(AApp * app) override;

		virtual void Shutdown() override;

		virtual void * GetUnderlyingPointer() override;

		virtual void ProcessMessages() override;

		virtual void CompleteFrame() override;

		virtual void ShowWindow() override;

		virtual bool KeepRunning() override;

		virtual void ShowDebugWindow() override;
	};

}

