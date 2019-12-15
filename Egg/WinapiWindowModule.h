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

	enum class MessageType : unsigned {
		DEVICE_LOST, RESIZED, FOCUSED, BLURRED
	};

	struct Message {
		MessageType type;
		union {
			DirectX::XMINT2 resizeDimensions;
		};
	};

	class WinApiAppEventSystem final : public AAppEventSystem {
		std::vector<Message> messages;
		bool isDeviceLost;

		void DispatchMsg(const Message & m, TAppEventHandler* handler);

	public:
		virtual void Dispatch() override;

		bool DeviceLost();
		void Post(const Message & m);
	};

	class WinapiWindowModule final : public IWindowModule {
		WinApiAppEventSystem eventSystem;
		HWND windowHandle;
		bool isRunning;

	public:
		virtual void Focused() override;
		virtual void Blurred() override;

		void Post(const Message & m);
		virtual void Start(AApp * app) override;
		virtual void Shutdown() override;
		virtual void * GetUnderlyingPointer() override;
		virtual void ProcessMessages() override;
		virtual void CompleteFrame() override;
		virtual bool KeepRunning() override;
		virtual void ShowWindow() override;
		virtual void ShowDebugWindow() override;
		virtual AAppEventSystem * GetEventSystem() override;
	};

}
