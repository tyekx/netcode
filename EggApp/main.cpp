#include <Egg/Common.h>
#include <Egg/App.h>
#include <Egg/Utility.h>
#include <chrono>
#include "EggApp.h"
#include <Egg/ProgramArgs.h>
#include <Egg/Path.h>
#include <shellapi.h>

std::unique_ptr<Egg::App> app{ nullptr };
HWND windowHandle;
UINT_PTR timerHandle = 0;
Egg::DisplayMode displayMode = Egg::DisplayMode::WINDOWED;
RECT lastWindowPos;

void SetDisplayMode(Egg::DisplayMode mode) {
	if(displayMode == mode) {
		return;
	}

	app->SetDisplayMode(mode);
	
	if(displayMode == Egg::DisplayMode::WINDOWED) {
		GetWindowRect(windowHandle, &lastWindowPos);

		SetWindowLong(windowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		com_ptr<IDXGIOutput> containingOutput = app->GetContainingOutput();

		DXGI_OUTPUT_DESC outputDesc;

		DX_API("Failed to get output desc")
			containingOutput->GetDesc(&outputDesc);

		SetWindowPos(
			windowHandle,
			HWND_TOPMOST,
			outputDesc.DesktopCoordinates.left,
			outputDesc.DesktopCoordinates.top,
			outputDesc.DesktopCoordinates.right,
			outputDesc.DesktopCoordinates.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(windowHandle, SW_MAXIMIZE);
	}

	if(mode == Egg::DisplayMode::WINDOWED) {
		// Restore the window's attributes and size.
		SetWindowLong(windowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);

		SetWindowPos(
			windowHandle,
			HWND_NOTOPMOST,
			lastWindowPos.left,
			lastWindowPos.top,
			lastWindowPos.right,
			lastWindowPos.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(windowHandle, SW_NORMAL);
	}

	displayMode = mode;
}


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
				app->Resize(width, height);
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
				if(displayMode == Egg::DisplayMode::WINDOWED) {
					SetDisplayMode(Egg::DisplayMode::FULLSCREEN);
				} else {
					SetDisplayMode(Egg::DisplayMode::WINDOWED);
				}
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
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR command, _In_ INT nShowCmd) {

	windowHandle = InitWindow(hInstance);
	// DirectX stuff
	com_ptr<ID3D12Debug3> debugController{ nullptr };
	com_ptr<IDXGIFactory6> dxgiFactory{ nullptr };
	com_ptr<IDXGISwapChain3> swapChain{ nullptr };
	com_ptr<ID3D12Device> device{ nullptr };
	com_ptr<ID3D12CommandQueue> commandQueue{ nullptr };
	 
	int argc;
	wchar_t ** args = CommandLineToArgvW(command, &argc);
	
	Egg::ProgramArgs pa{ (const wchar_t**)args, argc };

	if(!pa.IsSet(L"shaderPath") || !pa.IsSet(L"mediaPath")) {
		OutputDebugString("Invalid program arguments!\r\n");
		return 1;
	}

	Egg::Path::SetShaderRoot(pa.GetArg(L"shaderPath"));
	Egg::Path::SetMediaRoot(pa.GetArg(L"mediaPath"));

	// debug controller 
	DX_API("Failed to create debug layer")
		D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

	debugController->EnableDebugLayer();

	// needed to load WIC files (Windows Imaging Component) such as Jpg-s.
	DX_API("Failed to initialize COM library (ImportTexture)")
		CoInitialize(NULL);

	DX_API("Failed to create DXGI factory")
		CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));

	D3D12_FEATURE_DATA_FEATURE_LEVELS queryFeatureLevels;
	D3D_FEATURE_LEVEL possibleFeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1
	};

	//dxgiFactory->CheckFeatureSupport(, &queryFeatureLevels, sizeof(queryFeatureLevels));

	DX_API("Failed to create device")
		D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf()));
	
	DX_API("Failed to query feature support")
		device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &queryFeatureLevels, sizeof(D3D12_FEATURE_FEATURE_LEVELS));

	device.Reset();

	DX_API("Failed to create maximum feature level device")
		D3D12CreateDevice(nullptr, queryFeatureLevels.MaxSupportedFeatureLevel, IID_PPV_ARGS(device.GetAddressOf()));

	BOOL syncSupport = FALSE;

	DX_API("Failed to query sync support")
		dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &syncSupport, sizeof(syncSupport));

	// Create Command Queue

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	DX_API("Failed to create command queue")
		device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));

	// swap chain creation
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	// if you specify width/height as 0, the CreateSwapChainForHwnd will query it from the output window
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2; // back buffer depth
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = (syncSupport) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	com_ptr<IDXGISwapChain1> tempSwapChain;

	DX_API("Failed to create swap chain for HWND")
		dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), windowHandle, &swapChainDesc, NULL, NULL, tempSwapChain.GetAddressOf());

	DX_API("Failed to cast swap chain")
		tempSwapChain.As(&swapChain);

	DX_API("Failed to make window association") // disable ALT+Enter shortcut to full screen mode
		dxgiFactory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

	Egg::Input::CreateResources();

	app = std::make_unique<EggApp>();
	app->SetDevice(device);
	app->SetCommandQueue(commandQueue);
	app->SetSwapChain(swapChain);

	app->CreateSwapChainResources();
	app->CreateResources();
	app->LoadAssets();

	///app->SetDisplayMode(Egg::DisplayMode::BORDERLESS);

	ShowWindow(windowHandle, nShowCmd);

	MSG winMessage = { 0 };

	while(winMessage.message != WM_QUIT) {
		if(PeekMessage(&winMessage, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&winMessage);
			DispatchMessage(&winMessage);
		} else {
			app->Run();
		}
	}

	CoUninitialize();

	return 0;
}
