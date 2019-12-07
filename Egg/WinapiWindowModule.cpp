#include "WinapiWindowModule.h"
#include "Input.h"

namespace Egg::Module {

	LRESULT CALLBACK WindowProcess(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_DESTROY:
			// @TODO
			PostQuitMessage(0);
			return 0;

		case WM_SYSKEYDOWN:
			// Handle ALT+ENTER:
			if((wParam == VK_RETURN) && (lParam & (1 << 29)))
			{
				return 0;
			}
			break;

		case WM_NCLBUTTONDOWN:
			//timerHandle = SetTimer(windowHandle, 0, 16, TimerProcess);
			break;

		case WM_NCLBUTTONUP:
			//KillTimer(windowHandle, timerHandle);
			break;

		case WM_INPUT:
			Egg::Input::ReadRawMouse(wParam, lParam);
			break;
			//return 0;
		}

		return DefWindowProcW(windowHandle, message, wParam, lParam);
	}

	void WinapiWindowModule::CompleteFrame() {
		Egg::Input::Reset();
	}

	bool WinapiWindowModule::KeepRunning() {
		return isRunning;
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

		ShowWindow(wnd, SW_SHOWDEFAULT);

		isRunning = true;
	}

	void WinapiWindowModule::Shutdown() {
		PostQuitMessage(0);
	}

	void * WinapiWindowModule::GetUnderlyingPointer() {
		return reinterpret_cast<void *>(windowHandle);
	}

	void WinapiWindowModule::ProcessMessages() {
		MSG winMessage = { 0 };

		while(PeekMessage(&winMessage, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&winMessage);
			DispatchMessage(&winMessage);

			if(winMessage.message == WM_QUIT) {
				isRunning = false;
			}
		}
	}

}

