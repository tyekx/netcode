#pragma once



#include "Modules.h"
#include "Common.h"

#if defined(EGG_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace Egg::Module {

	class AApp;
	class IModule;
	class IWindowModule;

#define UM_RENDER (WM_USER + 0)

	class WinapiWindowModule final : public IWindowModule {
		UINT_PTR timerHandle;
		HWND windowHandle;
		bool isRunning;
	public:
		AppEventSystem * eventSystem;

		void EnterSizeMove();
		void ExitSizeMove();


		virtual void OnFocus() override;
		virtual void OnBlur() override;

		virtual void Start(AApp * app) override;
		virtual void Shutdown() override;
		virtual void * GetUnderlyingPointer() override;
		virtual void ProcessMessages() override;
		virtual void CompleteFrame() override;
		virtual bool KeepRunning() override;
		virtual void ShowWindow() override;
		virtual void ShowDebugWindow() override;
	};

}
