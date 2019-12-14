#include "WinapiWindowModule.h"
#include "Input.h"

namespace Egg::Module {

	LRESULT CALLBACK WindowProcess(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SYSKEYDOWN:
			if((wParam == VK_RETURN) && (lParam & (1 << 29)))
			{
				return 0;
			}
			break;

		case WM_ACTIVATE:
		case WM_SETFOCUS:
			Egg::Input::Focused();
			break;

		case WM_KILLFOCUS:
			Egg::Input::Blur();
			break;

		case WM_MOVE:
			OutputDebugString("move?\r\n");
			break;

		case WM_INPUT:
			Egg::Input::ReadRawMouse(wParam, lParam);
			return 0;

		case WM_KEYDOWN:
			Egg::Input::KeyPressed(wParam);
			break;

		case WM_KEYUP:
			Egg::Input::KeyReleased(wParam);
			break;
		}

		return DefWindowProc(windowHandle, message, wParam, lParam);
	}

	void WinapiWindowModule::CompleteFrame() {
		Egg::Input::Reset();
	}

	bool WinapiWindowModule::KeepRunning() {
		return isRunning;
	}

	void WinapiWindowModule::Start(AApp * app) {
		const char * windowClassName = "EggClass";


		WNDCLASSEX windowClass = { 0 };

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.lpfnWndProc = WindowProcess;
		windowClass.lpszClassName = windowClassName;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;

		RegisterClassEx(&windowClass);

		HWND wnd = CreateWindow(windowClassName,
								   "Netcode3D",
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
	
	void WinapiWindowModule::ShowWindow() {
		::ShowWindow(windowHandle, SW_SHOWDEFAULT);
	}

}

