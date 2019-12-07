#pragma once

#include "Modules.h"
#include "Common.h"

namespace Egg::Module {

	class AApp;
	class IModule;
	class IWindowModule;

	class WinapiWindowModule : public IWindowModule {
		HWND windowHandle;
		bool isRunning;

	public:
		virtual void Start(AApp * app) override;
		virtual void Shutdown() override;
		virtual void * GetUnderlyingPointer() override;
		virtual void ProcessMessages() override;
		virtual void CompleteFrame() override;
		virtual bool KeepRunning() override;
	};

}
