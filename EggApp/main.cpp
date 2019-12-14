#include <Egg/Common.h>
#include <Egg/App.h>
#include <Egg/Utility.h>
#include <chrono>
#include "EggApp.h"
#include <Egg/ProgramArgs.h>
#include <Egg/Path.h>
#include <shellapi.h>

/*
std::unique_ptr<Egg::App> app{ nullptr };
HWND windowHandle;
UINT_PTR timerHandle = 0;
Egg::DisplayMode displayMode = Egg::DisplayMode::WINDOWED;
RECT lastWindowPos;



void TimerProcess(HWND windowHandle, UINT a1, UINT_PTR a2, DWORD a3) {
	PostMessage(windowHandle, WM_PAINT, 0, 0);
}

LRESULT CALLBACK WindowProcess(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_DESTROY:
			app->Destroy();
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			if(app != nullptr) {
				int height = HIWORD(lParam);
				int width = LOWORD(lParam);
				//app->Resize(width, height);
			}
			return 0;

		case WM_KEYDOWN:
			app->KeyPressed(wParam);
			return 0;

		case WM_KEYUP: 
			app->KeyReleased(wParam);
			return 0;

		case WM_SYSKEYDOWN:
			// Handle ALT+ENTER:
			if((wParam == VK_RETURN) && (lParam & (1 << 29)))
			{
				return 0;
			}
			// Send all other WM_SYSKEYDOWN messages to the default WndProc.
			break;

		case WM_KILLFOCUS:
			app->Blur();
			return 0;

		case WM_ACTIVATE:
		case WM_SETFOCUS:
			app->Focused();
			return 0;

		case WM_NCLBUTTONDOWN:
			timerHandle = SetTimer(windowHandle, 0, 16, TimerProcess);
			break;

		case WM_NCLBUTTONUP:
			KillTimer(windowHandle, timerHandle);
			break;

		case WM_PAINT:
			app->Run();
			return 0;

		case WM_INPUT:
			Egg::Input::ReadRawMouse(wParam, lParam);
			return 0;
	}

	return DefWindowProcW(windowHandle, message, wParam, lParam);
}

HWND InitWindow(HINSTANCE hInstance) {
	const wchar_t * windowClassName = L"EggClass";

	WNDCLASSEXW windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEXW));

	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.lpszClassName = windowClassName;
	windowClass.hInstance = hInstance;
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
							   hInstance,
							   NULL);

	ASSERT(wnd != NULL, "Failed to create window");

	return wnd;
}*/

#include <Egg/DefaultModuleFactory.h>
#include <io.h>
#include <fcntl.h>

void SetupConsole() {
	AllocConsole();

	SetConsoleTitle("Netcode3D - Console");

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE * hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE * hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR command, _In_ INT nShowCmd) {
#if defined(_DEBUG)
	SetupConsole();
#endif

	Log::Setup(true);

	CoInitialize(nullptr);
	 
	int argc;
	wchar_t ** args = CommandLineToArgvW(command, &argc);
	
	Egg::ProgramArgs pa{ (const wchar_t**)args, argc };

	LocalFree(args);

	if(!pa.IsSet(L"shaderPath") || !pa.IsSet(L"mediaPath")) {
		Log::Error("Missing command arguments");
		return 1;
	}

	Egg::Path::SetShaderRoot(pa.GetArg(L"shaderPath"));
	Egg::Path::SetMediaRoot(pa.GetArg(L"mediaPath"));

	Egg::Input::CreateResources();

	Egg::Module::DefaultModuleFactory defModuleFactory;
	std::unique_ptr<Egg::Module::AApp> app = std::make_unique<GameApp>();
	app->Setup(&defModuleFactory);

	Log::Info("Initialization successful");

	app->Run();
	app->Exit();

	CoUninitialize();

	Log::Info("Gracefully shutting down");

	return 0;
}
