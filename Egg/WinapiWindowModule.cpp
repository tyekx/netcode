#include "WinapiWindowModule.h"
#include "Input.h"
#include <io.h>
#include <fcntl.h>

namespace Egg::Module {

	void SetupConsole() {

		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		int consoleHandleR, consoleHandleW;
		long stdioHandle;
		FILE * fptr;

		AllocConsole();
		std::string str = "Dev Console";
		SetConsoleTitle(str.c_str());

		EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_GRAYED);
		DrawMenuBar(GetConsoleWindow());

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo);

		stdioHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
		consoleHandleR = _open_osfhandle(stdioHandle, _O_TEXT);
		fptr = _fdopen(consoleHandleR, "r");
		*stdin = *fptr;
		setvbuf(stdin, NULL, _IONBF, 0);

		stdioHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
		consoleHandleW = _open_osfhandle(stdioHandle, _O_TEXT);
		fptr = _fdopen(consoleHandleW, "w");
		*stdout = *fptr;
		setvbuf(stdout, NULL, _IONBF, 0);

		stdioHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
		*stderr = *fptr;
		setvbuf(stderr, NULL, _IONBF, 0);

	}

	LRESULT CALLBACK WindowProcess(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
		WinapiWindowModule * pThis = reinterpret_cast<WinapiWindowModule *>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));

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
			if(pThis) {
				pThis->Post(Message{ MessageType::FOCUSED });
			}
			break;

		case WM_KILLFOCUS:
			if(pThis) {
				pThis->Post(Message{ MessageType::BLURRED });
			}
			break;

		case WM_INPUT:
			Egg::Input::ReadRawMouse(wParam, lParam);
			return 0;

		case WM_KEYDOWN:
			Egg::Input::KeyPressed(static_cast<UINT>(wParam));
			break;

		case WM_KEYUP:
			Egg::Input::KeyReleased(static_cast<UINT>(wParam));
			break;

		case WM_SIZE:
			if(pThis) {
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				Message m;
				m.type = MessageType::RESIZED;
				m.resizeDimensions.x = width;
				m.resizeDimensions.y = height;
				pThis->Post(m);
			}
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

	void WinapiWindowModule::Post(const Message & m) {
		eventSystem.Post(m);
	}

	void WinapiWindowModule::Start(AApp * app) {
		const char * windowClassName = "EggClass";


		#if defined(_DEBUG)
				ShowDebugWindow();
		#endif

		WNDCLASSEX windowClass = { 0 };

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.lpfnWndProc = WindowProcess;
		windowClass.lpszClassName = windowClassName;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.cbClsExtra = sizeof(void *);

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
								0);

		SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

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

	void WinapiWindowModule::ShowDebugWindow() {
		SetupConsole();
	}

	AAppEventSystem * WinapiWindowModule::GetEventSystem()
	{
		return &eventSystem;
	}

	void WinApiAppEventSystem::DispatchMsg(const Message & m, TAppEventHandler * handler) {
		switch(m.type) {
		case MessageType::BLURRED:
			handler->Blurred();
			break;
		case MessageType::DEVICE_LOST:
			handler->DeviceLost();
			break;
		case MessageType::FOCUSED:
			handler->Focused();
			break;
		case MessageType::RESIZED:
			handler->Resized(m.resizeDimensions.x, m.resizeDimensions.y);
			break;
		}
	}

	void WinApiAppEventSystem::Dispatch() {
		for(const auto & m : messages) {
			for(auto * handler : handlers) {
				DispatchMsg(m, handler);
			}
		}
		messages.clear();
	}

	bool WinApiAppEventSystem::DeviceLost()
	{
		return isDeviceLost;
	}

	void WinApiAppEventSystem::Post(const Message & m) {
		if(isDeviceLost) {
			return;
		}

		if(m.type == MessageType::DEVICE_LOST) {
			isDeviceLost = true;
			messages.clear();
		}

		messages.push_back(m);
	}

}

