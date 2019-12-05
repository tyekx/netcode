#include "WinapiWindowModule.h"

namespace Egg::Module {

	LRESULT CALLBACK WindowProcess(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		return DefWindowProcW(windowHandle, message, wParam, lParam);
	}


	void WinapiWindowModule::CompleteFrame() {

	}

	bool WinapiWindowModule::KeepRunning() {
		return true;
	}

	void WinapiWindowModule::Start(AApp * app) {
		const wchar_t * windowClassName = L"EggClass";

		WNDCLASSEXW windowClass;
		ZeroMemory(&windowClass, sizeof(WNDCLASSEXW));

		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.lpfnWndProc = WindowProcess;
		windowClass.lpszClassName = windowClassName;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;

		RegisterClassExW(&windowClass);

		HWND wnd = CreateWindowExW(0,
								   windowClassName,
								   L"Netcode3D",
								   WS_OVERLAPPEDWINDOW,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   CW_USEDEFAULT,
								   NULL,
								   NULL,
								   windowClass.hInstance,
								   NULL);

		ASSERT(wnd != NULL, "Failed to create window");

		windowHandle = wnd;
	}

	void WinapiWindowModule::Shutdown() {
		PostQuitMessage(0);
	}

	void * WinapiWindowModule::GetUnderlyingPointer() {
		return reinterpret_cast<void *>(windowHandle);
	}

	void WinapiWindowModule::ProcessMessages() {
		/*peek&translate, set values to Input class*/
	}

}

