#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include "FBXImporter.h"
#include "EditorModuleFactory.h"
#include <Netcode/Input.h>
#include <Netcode/IO/Path.h>

#include "CreateColliderDialog.h"

#include "BonesPage.h"
#include "AnimationsPage.h"
#include "CollidersPage.h"
#include "GeometryPage.h"
#include "MaterialsPage.h"
#include "NetcodeAssetExporter.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    void MainPage::RenderTimer_OnTick(Windows::Foundation::IInspectable const & sender, Windows::Foundation::IInspectable const & e) {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        if(Global::EditorApp != nullptr) {
            Global::EditorApp->Run();
        }
    }

    MainPage::MainPage() {
        InitializeComponent();

        renderTimer.Interval(std::chrono::milliseconds{ 16 });
        renderTimer.Tick(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>(this, &MainPage::RenderTimer_OnTick));

        pointerHeld = false;
    }

    winrt::event_token MainPage::ManifestChanged(NetcodeAssetEditor::ManifestChangedHandler const & handler)
    {
        return manifestChanged.add(handler);
    }

    void MainPage::ManifestChanged(winrt::event_token const & token) noexcept
    {
        manifestChanged.remove(token);
    }

    winrt::event_token MainPage::ModelChanged(NetcodeAssetEditor::ModelChangedHandler const & handler)
    {
        return modelChanged.add(handler);
    }

    void MainPage::ModelChanged(winrt::event_token const & token) noexcept
    {
        modelChanged.remove(token);
    }

    void MainPage::FileCtx_CreateManifest_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        auto lifetime = get_strong();

        if(Global::Manifest != nullptr) {
            // close
            throw hresult_not_implemented();
        }

        Global::Manifest = std::make_unique<Netcode::Asset::Manifest>();

        //manifestChanged(reinterpret_cast<uint64_t>(manifest.get()));
    }

    void MainPage::CommandInvokeHandler(Windows::UI::Popups::IUICommand const & command)
    {
        UNREFERENCED_PARAMETER(command);

        // ok
    }

    void MainPage::swapChainPanel_Loaded(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);


        Netcode::Module::EditorModuleFactory editorModuleFactory;

        Global::Model = std::make_unique<Model>();
        Global::EditorApp = std::make_unique<Netcode::Module::EditorApp>();
        Global::EditorApp->Setup(&editorModuleFactory);

        auto nativeSwapChainPanel = swapChainPanel().as<ISwapChainPanelNative>();

        HRESULT hr = nativeSwapChainPanel->SetSwapChain(reinterpret_cast<IDXGISwapChain *>(Global::EditorApp->graphics->GetSwapChain()));

        auto f2 = swapChainPanel().ActualSize();

        Netcode::Module::AppEvent appEvent;
        appEvent.resizeArgs = Netcode::Int2{ static_cast<int32_t>(f2.x), static_cast<int32_t>(f2.y) };
        appEvent.type = Netcode::Module::EAppEventType::RESIZED;
        Global::EditorApp->events->Broadcast(appEvent);

        if(SUCCEEDED(hr)) {
            renderTimer.Start();
        }
    }


    void MainPage::swapChainPanel_SizeChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::SizeChangedEventArgs const & e) {
        if(Global::EditorApp != nullptr) {
            auto newSize = e.NewSize();
            Netcode::Module::AppEvent appEvent;
            appEvent.resizeArgs = Netcode::Int2{ static_cast<int32_t>(newSize.Width), static_cast<int32_t>(newSize.Height) };
            appEvent.type = Netcode::Module::EAppEventType::RESIZED;
            Global::EditorApp->events->PostEvent(appEvent);
        }
    }


    void MainPage::Geometry_ListView_SelectAll() {
        //geometryListView().SelectAll();
    }

    /*
    Saves the cursor at screenspace position, then sets the cursor to the middle of the swap chain and hides it, this is to prepare for the dragging effect
    */
    void MainPage::swapChainPanel_PointerPressed(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        DataContext().as<DC_MainPage>()->BoxedTestFloat4(box_value(Windows::Foundation::Numerics::float4{ 1.0f, 2.0f, 3.0f, 4.0f }));

        pointerHeld = true;
        Windows::Foundation::Numerics::float2 swapChainActualSize = swapChainPanel().ActualSize();

        auto windowSpaceTransform = swapChainPanel().TransformToVisual(Windows::UI::Xaml::Window::Current().Content());
        auto bounds = Windows::UI::Xaml::Window::Current().Bounds();
        auto pointerPressPosition_PanelSpace = e.GetCurrentPoint(swapChainPanel()).Position();
        auto pointerPressPosition_WindowSpace = windowSpaceTransform.TransformPoint(pointerPressPosition_PanelSpace);
        auto swapChainCenter_PanelSpace = Windows::Foundation::Point{ swapChainActualSize.x / 2.0f, swapChainActualSize.y / 2.0f };
        auto swapChainCenter_WindowSpace = windowSpaceTransform.TransformPoint(swapChainCenter_PanelSpace);

        swapChain_PointerPressedAt_ScreenSpace = Windows::Foundation::Point{ bounds.X + pointerPressPosition_WindowSpace.X,
                                                                             bounds.Y + pointerPressPosition_WindowSpace.Y };

        swapChain_LastPointerPosition_PanelSpace = swapChainCenter_PanelSpace;

        Windows::UI::Xaml::Window::Current().CoreWindow().PointerPosition(Windows::Foundation::Point{ bounds.X + swapChainCenter_WindowSpace.X,
                                                                                                      bounds.Y + swapChainCenter_WindowSpace.Y });
        Windows::UI::Xaml::Window::Current().CoreWindow().PointerCursor(nullptr);
    }

    void MainPage::swapChainPanel_PointerReleased(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        pointerHeld = false;
        Windows::UI::Xaml::Window::Current().CoreWindow().PointerCursor(Windows::UI::Core::CoreCursor{ Windows::UI::Core::CoreCursorType::Arrow, 1 });
        Windows::UI::Xaml::Window::Current().CoreWindow().PointerPosition(swapChain_PointerPressedAt_ScreenSpace);
    }

    void MainPage::swapChainPanel_PointerMoved(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);

        if(pointerHeld) {
            auto pointerMovePosition_PanelSpace = e.GetCurrentPoint(swapChainPanel()).Position();
            auto windowSpaceTransform = swapChainPanel().TransformToVisual(Windows::UI::Xaml::Window::Current().Content());

            Windows::Foundation::Numerics::float2 swapChainActualSize = swapChainPanel().ActualSize();
            auto swapChainCenter_PanelSpace = Windows::Foundation::Point{ swapChainActualSize.x / 2.0f, swapChainActualSize.y / 2.0f };
            auto swapChainCenter_WindowSpace = windowSpaceTransform.TransformPoint(swapChainCenter_PanelSpace);
            auto bounds = Windows::UI::Xaml::Window::Current().Bounds();

            auto delta = Windows::Foundation::Point{ pointerMovePosition_PanelSpace.X - swapChainCenter_PanelSpace.X,
                                                     pointerMovePosition_PanelSpace.Y - swapChainCenter_PanelSpace.Y };

            Global::EditorApp->MouseMoved(DirectX::XMFLOAT2{ delta.X, delta.Y });

            Windows::UI::Xaml::Window::Current().CoreWindow().PointerPosition(Windows::Foundation::Point{ bounds.X + swapChainCenter_WindowSpace.X,
                                                                                                          bounds.Y + swapChainCenter_WindowSpace.Y });

            swapChain_LastPointerPosition_PanelSpace = swapChainCenter_PanelSpace;

            Global::EditorApp->InvalidateFrame();
        }
    }

    void MainPage::Pivot_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        uint32_t selectionIndex = mainPivot().SelectedIndex();

        switch(selectionIndex) {
            case 0: mainFrame().Navigate(xaml_typename<NetcodeAssetEditor::GeometryPage>(), *this); break;
            case 1: mainFrame().Navigate(xaml_typename<NetcodeAssetEditor::MaterialsPage>(), *this); break;
            case 2: mainFrame().Navigate(xaml_typename<NetcodeAssetEditor::BonesPage>(), *this); break;
            case 3: mainFrame().Navigate(xaml_typename<NetcodeAssetEditor::AnimationsPage>(), *this); break;
            case 4: mainFrame().Navigate(xaml_typename<NetcodeAssetEditor::CollidersPage>(), *this); break;
        }
    }

    fire_and_forget MainPage::AssetCtx_ImportFBX_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        auto lifetime = get_strong();

        Windows::Storage::Pickers::FileOpenPicker fileOpenPicker;
        fileOpenPicker.FileTypeFilter().ReplaceAll({ L".fbx" });
        fileOpenPicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);


        Windows::Storage::StorageFile file = co_await fileOpenPicker.PickSingleFileAsync();

        if(file != nullptr) {
            Windows::Storage::Streams::IRandomAccessStream readStream = co_await file.OpenAsync(Windows::Storage::FileAccessMode::Read);

            uint64_t size = readStream.Size();

            if(size > 0xFFFFFFFFull) {
                throw hresult_out_of_bounds();
            }

            Windows::Storage::Streams::IBuffer buffer = Windows::Storage::Streams::Buffer{ static_cast<uint32_t>(size) };

            buffer = co_await readStream.ReadAsync(buffer, static_cast<uint32_t>(size), Windows::Storage::Streams::InputStreamOptions::None);

            *Global::Model = FBXImporter::FromMemory(buffer.data(), buffer.Length());

            std::vector<DirectX::BoundingBox> boundingBoxes;
            boundingBoxes.reserve(Global::Model->meshes.size());
            for(Mesh & m : Global::Model->meshes) {
                boundingBoxes.push_back(m.boundingBox);
            }
            Global::EditorApp->SetBoundingBoxes(std::move(boundingBoxes));

            auto dcMainPage = DataContext().as<DC_MainPage>();

            dcMainPage->Meshes().Clear();
            for(const Mesh & mesh : Global::Model->meshes) {
                auto dcMesh = winrt::make<DC_Mesh>();
                dcMesh.Name(winrt::to_hstring(mesh.name));

                for(const LOD & lod : mesh.lods) {
                    dcMesh.LodLevels().Append(winrt::make<DC_Lod>(lod.vertexCount, lod.indexCount, 0, 0, lod.vertexDataSizeInBytes, lod.indexDataSizeInBytes));
                }

                dcMainPage->Meshes().Append(dcMesh);
            }

            Global::EditorApp->SetMaterials(Global::Model->meshes, Global::Model->materials);

            modelChanged(reinterpret_cast<uint64_t>(Global::Model.get()));

            if(Global::Manifest == nullptr) {
                Global::Manifest = std::make_unique<Netcode::Asset::Manifest>();
            }

            Global::Manifest->base.file = winrt::to_string(file.Path());

            Geometry_ListView_SelectAll();
        }
    }

    winrt::fire_and_forget MainPage::AssetCtx_ImportAnimation_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        auto lifetime = get_strong();

        if(Global::Manifest == nullptr) {
            return;
        }

        Windows::Storage::Pickers::FileOpenPicker fileOpenPicker;
        fileOpenPicker.FileTypeFilter().ReplaceAll({ L".fbx" });
        
        fileOpenPicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFile> files = co_await fileOpenPicker.PickMultipleFilesAsync();

        if(files != nullptr) {
            loadingScreen().Visibility(Windows::UI::Xaml::Visibility::Visible);
            loadingScreenTitle().Text(L"Processed 0/" + to_hstring(files.Size()) + L" file(s)");
            for(uint32_t i = 0; i < files.Size(); ++i) {
                Windows::Storage::StorageFile file = files.GetAt(i);

                std::string animationName = winrt::to_string(file.DisplayName());

                Windows::Storage::Streams::IRandomAccessStream readStream = co_await file.OpenAsync(Windows::Storage::FileAccessMode::Read);

                uint64_t size = readStream.Size();

                if(size > 0xFFFFFFFFull) {
                    throw hresult_out_of_bounds();
                }

                Windows::Storage::Streams::IBuffer buffer = Windows::Storage::Streams::Buffer{ static_cast<uint32_t>(size) };

                buffer = co_await readStream.ReadAsync(buffer, static_cast<uint32_t>(size), Windows::Storage::Streams::InputStreamOptions::None);

                auto imported = FBXImporter::ImportAnimationsFromMemory(buffer.data(), buffer.Length(), Global::Model->skeleton);
                
                auto optAnim = FBXImporter::OptimizeAnimation(imported.at(0), Global::Model->skeleton);
                optAnim.name = animationName;

                Global::Model->animations.push_back(std::move(optAnim));

                Netcode::Asset::Manifest::Animation anim;
                anim.name = animationName;
                anim.editorPlaybackSpeed = 1.0f;
                anim.editorPlaybackLoop = true;
                anim.source.file = winrt::to_string(file.Path());
                anim.source.reference = "0";
                Global::Manifest->animations.push_back(std::move(anim));

                loadingScreenTitle().Text(L"Processed " + to_hstring(i + 1) +  L"/" + to_hstring(files.Size()) + L" file(s)");
            }
        }

        loadingScreen().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
    }


    winrt::fire_and_forget MainPage::FileCtx_SaveManifest_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        auto lifetime = get_strong();

        if(Global::Manifest == nullptr || Global::Model == nullptr) {
            return;
        }

        Windows::Storage::Pickers::FileSavePicker fileSavePicker;
        fileSavePicker.SuggestedFileName(L"manifest");
        fileSavePicker.FileTypeChoices().Insert(L"JSON", winrt::single_threaded_vector<hstring>({ L".json" }));
        fileSavePicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        Windows::Storage::StorageFile file = co_await fileSavePicker.PickSaveFileAsync();

        if(file != nullptr) {
            Windows::Storage::CachedFileManager::DeferUpdates(file);

            Global::Manifest->colliders.clear();
            Global::Manifest->colliders = Global::Model->colliders;

            Global::Manifest->materials;

            co_await Windows::Storage::FileIO::WriteTextAsync(file, winrt::to_hstring(Global::Manifest->Store().dump()));

            Windows::Storage::Provider::FileUpdateStatus status = co_await Windows::Storage::CachedFileManager::CompleteUpdatesAsync(file);

            if(status == Windows::Storage::Provider::FileUpdateStatus::Complete) {
                Windows::UI::Popups::MessageDialog msg{ L"Manifest was successfully saved at: " + file.Path(), L"Success" };
                Windows::UI::Popups::UICommand closeCommand{ L"Close", { this, &MainPage::CommandInvokeHandler } };

                msg.Commands().Append(closeCommand);

                msg.ShowAsync();
            }
        }
    }

    fire_and_forget MainPage::FileCtx_LoadManifest_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        auto lifetime = get_strong();

        Windows::Storage::Pickers::FileOpenPicker fileOpenPicker;

        fileOpenPicker.FileTypeFilter().ReplaceAll({ L".json" });
        fileOpenPicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        loadingScreen().Visibility(Windows::UI::Xaml::Visibility::Visible);
        loadingScreenTitle().Text(L"Waiting for manifest...");

        Windows::Storage::StorageFile file = co_await fileOpenPicker.PickSingleFileAsync();

        if(file != nullptr) {

            loadingScreenTitle().Text(L"Loading " + file.DisplayName() + L"...");

            Windows::Storage::Streams::IRandomAccessStream readStream = co_await file.OpenAsync(Windows::Storage::FileAccessMode::Read);

            uint64_t size = readStream.Size();

            if(size > 0xFFFFFFFFull) {
                throw hresult_out_of_bounds();
            }

            Windows::Storage::Streams::IBuffer buffer = Windows::Storage::Streams::Buffer{ static_cast<uint32_t>(size) };

            buffer = co_await readStream.ReadAsync(buffer, static_cast<uint32_t>(size), Windows::Storage::Streams::InputStreamOptions::None);

            std::string str;
            str.assign(reinterpret_cast<char *>(buffer.data()), static_cast<size_t>(buffer.Length()));

            std::string err;
            json11::Json json = json11::Json::parse(str, err);

            if(Global::Manifest == nullptr) {
                Global::Manifest = std::make_unique<Netcode::Asset::Manifest>();
            }

            if(!Global::Manifest->Load(json)) {
                OutputDebugStringW(L"Not ok\r\n");
            }

            if(Global::Model == nullptr) {
                Global::Model = std::make_unique<Model>();
            }

            Windows::Storage::StorageFile baseFile = co_await Windows::Storage::StorageFile::GetFileFromPathAsync(to_hstring(Global::Manifest->base.file));

            loadingScreenTitle().Text(L"Loading " + baseFile.DisplayName() + L".fbx...");

            Windows::Storage::Streams::IRandomAccessStream baseReadStream = co_await baseFile.OpenAsync(Windows::Storage::FileAccessMode::Read);

            Windows::Storage::Streams::IBuffer baseBuffer = Windows::Storage::Streams::Buffer{ static_cast<uint32_t>(baseReadStream.Size()) };

            baseBuffer = co_await baseReadStream.ReadAsync(baseBuffer, static_cast<uint32_t>(baseReadStream.Size()), Windows::Storage::Streams::InputStreamOptions::None);

            *Global::Model = FBXImporter::FromMemory(baseBuffer.data(), baseBuffer.Length());

            for(const auto & animRef : Global::Manifest->animations) {
                hstring animFile = to_hstring(animRef.source.file);

                Windows::Storage::StorageFile animationFile = co_await Windows::Storage::StorageFile::GetFileFromPathAsync(animFile);

                loadingScreenTitle().Text(L"Loading " + animationFile.DisplayName() + L".fbx...");

                Windows::Storage::Streams::IRandomAccessStream animReadStream = co_await animationFile.OpenAsync(Windows::Storage::FileAccessMode::Read);

                Windows::Storage::Streams::IBuffer animBuffer = Windows::Storage::Streams::Buffer{ static_cast<uint32_t>(animReadStream.Size()) };

                animBuffer = co_await animReadStream.ReadAsync(animBuffer, static_cast<uint32_t>(animReadStream.Size()), Windows::Storage::Streams::InputStreamOptions::None);

                auto anim = FBXImporter::ImportAnimationsFromMemory(animBuffer.data(), animBuffer.Length(), Global::Model->skeleton);
                auto optimizedAnim = FBXImporter::OptimizeAnimation(anim.front(), Global::Model->skeleton);
                optimizedAnim.name = animRef.name;

                Global::Model->animations.push_back(std::move(optimizedAnim));
            }

            Global::Model->colliders = Global::Manifest->colliders;
            Global::EditorApp->SetColliders(Global::Model->colliders);

            std::vector<DirectX::BoundingBox> boundingBoxes;
            boundingBoxes.reserve(Global::Model->meshes.size());
            for(Mesh & m : Global::Model->meshes) {
                boundingBoxes.push_back(m.boundingBox);
            }
            Global::EditorApp->SetBoundingBoxes(std::move(boundingBoxes));

            auto dcMainPage = DataContext().as<DC_MainPage>();

            dcMainPage->Meshes().Clear();
            for(const Mesh & mesh : Global::Model->meshes) {
                auto dcMesh = winrt::make<DC_Mesh>();
                dcMesh.Name(winrt::to_hstring(mesh.name));

                for(const LOD & lod : mesh.lods) {
                    dcMesh.LodLevels().Append(winrt::make<DC_Lod>(lod.vertexCount, lod.indexCount, 0, 0, lod.vertexDataSizeInBytes, lod.indexDataSizeInBytes));
                }

                dcMainPage->Meshes().Append(dcMesh);
            }

        }

        loadingScreen().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
    }

    winrt::fire_and_forget MainPage::FileCtx_Compile_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        Windows::Storage::Pickers::FileSavePicker fileSavePicker;

        fileSavePicker.SuggestedFileName(L"asset");
        fileSavePicker.FileTypeChoices().Insert(L"NetcodeAsset", winrt::single_threaded_vector<hstring>({ L".ncasset" }));
        fileSavePicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        Windows::Storage::StorageFile file = co_await fileSavePicker.PickSaveFileAsync();

        if(file != nullptr) {
            Windows::Storage::CachedFileManager::DeferUpdates(file);

            hstring path = file.Path();

            auto [rawData, size] = NetcodeAssetExporter::Export(*Global::Model);

            winrt::array_view<const uint8_t> arrayView( rawData.get(), rawData.get() + size );

            co_await Windows::Storage::FileIO::WriteBytesAsync(file, arrayView);

            Windows::Storage::Provider::FileUpdateStatus status = co_await Windows::Storage::CachedFileManager::CompleteUpdatesAsync(file);

            if(status == Windows::Storage::Provider::FileUpdateStatus::Complete) {
                Windows::UI::Popups::MessageDialog msg{ L"Asset was successfully saved at: " + file.Path(), L"Success" };
                Windows::UI::Popups::UICommand closeCommand{ L"Close", { this, &MainPage::CommandInvokeHandler } };

                msg.Commands().Append(closeCommand);

                msg.ShowAsync();
            }
        }
    }


    winrt::fire_and_forget MainPage::AssetCtx_SetMediaRoot_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e) {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        namespace st = Windows::Storage;

        st::Pickers::FolderPicker folderPicker;

        folderPicker.FileTypeFilter().ReplaceAll({ L".ncasset" });
        st::StorageFolder folder = co_await folderPicker.PickSingleFolderAsync();

        if(folder != nullptr) {
            std::wstring path = folder.Path().c_str();

            Netcode::IO::Path::FixDirectoryPath(path);

            Netcode::IO::Path::SetMediaRoot(path);
        }
    }

}
