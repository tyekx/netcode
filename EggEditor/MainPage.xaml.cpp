//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <Egg/ProgramArgs.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include "UC_GameObject.xaml.h"
#include "UC_ScriptComponent.xaml.h"
#include "UC_TransformComponent.xaml.h"
#include "UC_MeshComponent.xaml.h"
#include "UC_AddComponent.xaml.h"
#include "UC_Asset.xaml.h"
#include "UC_ProjectFolder.xaml.h"
#include "UC_BreadCrumb.xaml.h"
#include "UC_Scene.xaml.h"
#include "UC_Properties.xaml.h"
#include "UC_AssetBrowser.xaml.h"

using namespace EggEditor;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

inline int ConvertToPixels(float dimension, float dpi)
{
	return static_cast<int>(dimension * dpi / 96.0f + 0.5f);
}

MainPage::MainPage()
{
	InitializeComponent();
	totalTime = 0.0f;

	sceneControl->GameObjectSelected += ref new EggEditor::GameObjectSelectedCallback(propertiesControl, &UC_Properties::OnGameObjectSelected);
}


void EggEditor::MainPage::OnOpenAsset(Platform::Object ^ asset) {
	MainPageDataContext ^ dc = (MainPageDataContext ^)DataContext;
	dc->MainWindowIndex = 1;
}


void EggEditor::MainPage::Page_Loaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	timer = ref new DispatcherTimer;
	gameApp = std::make_unique<ggl005App>();

	Windows::Graphics::Display::DisplayInformation^ di = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
	float dpi = di->LogicalDpi;

	// DirectX stuff
	com_ptr<ID3D12Debug> debugController{ nullptr };
	com_ptr<IDXGIFactory6> dxgiFactory{ nullptr };
	com_ptr<IDXGISwapChain3> swapChain{ nullptr };
	com_ptr<ID3D12Device> device{ nullptr };
	com_ptr<ID3D12CommandQueue> commandQueue{ nullptr };

	//C:\work\directx12\Bin\Shaders
	const wchar_t * cstrArgs[] = {
	 L"--mediaPath=C:\\work\\directx12\\Bin\\AppX\\Media\\",
	 L"--shaderPath=C:\\work\\directx12\\Bin\\AppX\\Shaders"
	};
	Egg::ProgramArgs programArgs{ cstrArgs, _countof(cstrArgs) };

	ASSERT(programArgs.IsSet(L"mediaPath") && programArgs.IsSet(L"shaderPath"), "Invalid launch arguments, must set media and shader paths");

	Egg::Path::SetMediaRoot(programArgs.GetArg(L"mediaPath"));
	Egg::Path::SetShaderRoot(programArgs.GetArg(L"shaderPath"));

	// debug controller 
	DX_API("Failed to create debug layer")
		D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

	debugController->EnableDebugLayer();

	// needed to load WIC files (Windows Imaging Component) such as Jpg-s.
	//DX_API("Failed to initialize COM library (ImportTexture)")
		//CoInitialize(NULL);

	DX_API("Failed to create DXGI factory")
		CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));

	std::vector<com_ptr<IDXGIAdapter1>> adapters;
	//Egg::Utility::GetAdapters(dxgiFactory.Get(), adapters);

	// select your adapter here, NULL = system default
	IUnknown * selectedAdapter = (adapters.size() > 0) ? adapters[0].Get() : NULL;

	DX_API("Failed to create D3D Device")
		D3D12CreateDevice(selectedAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf()));

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
	swapChainDesc.Width = ConvertToPixels(swpWidth, dpi);
	swapChainDesc.Height = ConvertToPixels(swpHeight, dpi);
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2; // back buffer depth
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.Flags = 0;

	com_ptr<IDXGISwapChain1> tempSwapChain;

	DX_API("Failed to create swap chain")
		dxgiFactory->CreateSwapChainForComposition(commandQueue.Get(), &swapChainDesc, nullptr, tempSwapChain.GetAddressOf());

	DX_API("Failed to cast swap chain")
		tempSwapChain.As(&swapChain);

	com_ptr<ISwapChainPanelNative> uwpNativeSwapChain;
	IInspectable * panelInspectable = reinterpret_cast<IInspectable *>(swapChainPanel);

	DX_API("Failed to query interface")
	panelInspectable->QueryInterface(IID_PPV_ARGS(uwpNativeSwapChain.GetAddressOf()));

	DX_API("Failed to set swap chain")
	uwpNativeSwapChain->SetSwapChain(swapChain.Get());

	gameApp->SetDevice(device);
	gameApp->SetSwapChain(swapChain);
	gameApp->SetCommandQueue(commandQueue);

	gameApp->CreateResources();
	gameApp->CreateSwapChainResources();
	gameApp->LoadAssets();

	Windows::Foundation::TimeSpan ts;
	ts.Duration = 16;

	timer->Tick += ref new Windows::Foundation::EventHandler<Platform::Object^>(this, &EggEditor::MainPage::DispatcherTimer_Tick);
	timer->Interval = ts;

	gameApp->Update(0.0f, 0.0f);
	gameApp->Render();

	gameApp->SetScene(std::make_unique<Egg::Scene>());
	sceneControl->SetScene(reinterpret_cast<UINT_PTR>(gameApp->GetScene()));
	
}

void EggEditor::MainPage::DispatcherTimer_Tick(Platform::Object ^ sender, Platform::Object ^ e) {
	float dt = stopwatch.Restart();
	totalTime += dt;
	gameApp->Update(dt, totalTime);
	gameApp->Render();
	propertiesControl->Update();
}


void EggEditor::MainPage::SwapChainPanel_SizeChanged(Platform::Object ^ sender, Windows::UI::Xaml::SizeChangedEventArgs ^ e)
{
	swpHeight = e->NewSize.Height;
	swpWidth = e->NewSize.Width;

	if(gameApp) {
		gameApp->Resize(ConvertToPixels(swpWidth, 96.0f), ConvertToPixels(swpHeight, 96.0f));
	}
}

void EggEditor::MainPage::SwapChainPanel_PointerPressed(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ e)
{
	Windows::UI::Input::PointerPoint^pp = e->GetCurrentPoint(swapChainPanel);

}


void EggEditor::MainPage::BtnOutput_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	MainPageDataContext ^ dc = (MainPageDataContext ^)DataContext;
	dc->SubWindowIndex = 0;
}


void EggEditor::MainPage::BtnAssets_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	MainPageDataContext ^ dc = (MainPageDataContext ^)DataContext;
	dc->SubWindowIndex = 1;
}


void EggEditor::MainPage::BtnPreview_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	MainPageDataContext ^ dc = (MainPageDataContext ^)DataContext;
	dc->MainWindowIndex = 0;
}


void EggEditor::MainPage::BtnAssetEditor_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	MainPageDataContext ^ dc = (MainPageDataContext ^)DataContext;
	dc->MainWindowIndex = 1;
}

void EggEditor::MainPage::BtnStart_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	stopwatch.Start();
	timer->Start();
}

void EggEditor::MainPage::BtnPause_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	timer->Stop();
	stopwatch.Stop();
}

void EggEditor::MainPage::BtnReset_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	totalTime = 0.0f;
	timer->Stop();
	stopwatch.Reset();
	gameApp->Render();
}

void EggEditor::MainPage::EditMenuBtnImport_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	auto filePicker = ref new Windows::Storage::Pickers::FileOpenPicker();
	filePicker->FileTypeFilter->Append(L".jpg");
	filePicker->FileTypeFilter->Append(L".png");
	filePicker->FileTypeFilter->Append(L".jpeg");
	filePicker->FileTypeFilter->Append(L".obj");
	auto asyncOpenFiles = filePicker->PickMultipleFilesAsync();

	auto openTask = concurrency::create_task(asyncOpenFiles);

	openTask.then([this](Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFile ^ > ^ result) -> void {
		for(unsigned int i = 0; i < result->Size; ++i) {
			assetBrowserControl->ImportAsset(result->GetAt(i));
		}
	});
}

void EggEditor::MainPage::FileMenuBtnNew_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e) {
	auto picker = ref new Windows::Storage::Pickers::FileSavePicker();
	auto fileTypePostfixes = ref new Platform::Collections::Vector<Platform::String^>();
	fileTypePostfixes->Append(L".eggproj");
	picker->FileTypeChoices->Insert(L"Egg Project", fileTypePostfixes);
	picker->SuggestedFileName = L"newproj";

	auto asyncSaveFile = picker->PickSaveFileAsync();

	auto saveTask = concurrency::create_task(asyncSaveFile);

	saveTask.then([this](Windows::Storage::StorageFile^ result) -> void {
		if(result != nullptr) {
			Windows::Storage::CachedFileManager::DeferUpdates(result);

			auto asyncWriteFile = Windows::Storage::FileIO::WriteTextAsync(result, L"{}\r\n");

			auto writeTask = concurrency::create_task(asyncWriteFile);
			writeTask.then([this, result]() -> void {
				auto asyncUpdateFile = Windows::Storage::CachedFileManager::CompleteUpdatesAsync(result);
				auto updateTask = concurrency::create_task(asyncUpdateFile);

				updateTask.then([this, result](Windows::Storage::Provider::FileUpdateStatus updateStatus) -> void {
					if(updateStatus == Windows::Storage::Provider::FileUpdateStatus::Complete) {
						OutputDebugString(L"File successfully saved\r\n");
					} else {
						OutputDebugString(L"Failed to save file\r\n");
					}
				});
			});
		}

    });

}
