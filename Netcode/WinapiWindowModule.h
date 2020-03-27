#pragma once


#if defined(EGG_OS_WINDOWS)
#include <Windows.h>
#endif

#include "Modules.h"
#include "Common.h"


namespace Netcode::Module {

	class AApp;
	class IModule;
	class IWindowModule;

#define UM_RENDER (WM_USER + 0)

	class WinapiWindowModule final : public IWindowModule {
		UINT_PTR timerHandle;
		HWND windowHandle;
		bool isRunning;
		RECT lastWindowedPos;
		RECT lastWindowedClientRect;
		UINT windowedStyle;
		Netcode::Module::IGraphicsModule * graphics;
	public:
		Netcode::Graphics::DisplayMode displayMode;
		AppEventSystem * eventSystem;

		void EnterSizeMove();
		void ExitSizeMove();


		virtual void OnFocus() override;
		virtual void OnBlur() override;
		virtual void OnModeChanged(Netcode::Graphics::DisplayMode mode) override;

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
