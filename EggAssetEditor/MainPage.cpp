#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include "FBXImporter.h"
#include "EditorModuleFactory.h"
#include <Egg/Input.h>

#include "CreateColliderDialog.h"

#include "BonesPage.h"
#include "AnimationsPage.h"
#include "CollidersPage.h"
#include "GeometryPage.h"
#include "MaterialsPage.h"

namespace winrt::EggAssetEditor::implementation
{
    MainPage::MainPage() {
        
        InitializeComponent();

        pointerHeld = false;
    }

    winrt::event_token MainPage::ManifestChanged(EggAssetEditor::ManifestChangedHandler const & handler)
    {
        return manifestChanged.add(handler);
    }

    void MainPage::ManifestChanged(winrt::event_token const & token) noexcept
    {
        manifestChanged.remove(token);
    }

    winrt::event_token MainPage::ModelChanged(EggAssetEditor::ModelChangedHandler const & handler)
    {
        return modelChanged.add(handler);
    }

    void MainPage::ModelChanged(winrt::event_token const & token) noexcept
    {
        modelChanged.remove(token);
    }

    void MainPage::FileCtx_CreateManifest_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        auto lifetime = get_strong();

        if(Global::Manifest != nullptr) {
            // close
            throw hresult_not_implemented();
        }

        Global::Manifest = std::make_unique<Manifest>();

        //manifestChanged(reinterpret_cast<uint64_t>(manifest.get()));
    }

    void MainPage::swapChainPanel_Loaded(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        Egg::Module::EditorModuleFactory editorModuleFactory;

        Global::Model = std::make_unique<Model>();
        Global::EditorApp = std::make_unique<Egg::Module::EditorApp>();
        Global::EditorApp->Setup(&editorModuleFactory);

        auto nativeSwapChainPanel = swapChainPanel().as<ISwapChainPanelNative>();

        HRESULT hr = nativeSwapChainPanel->SetSwapChain(reinterpret_cast<IDXGISwapChain *>(Global::EditorApp->graphics->GetSwapChain()));

        if(SUCCEEDED(hr)) {
            OutputDebugStringW(L"Ok\r\n");
            Global::EditorApp->Run();
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
        pointerHeld = false;
        Windows::UI::Xaml::Window::Current().CoreWindow().PointerCursor(Windows::UI::Core::CoreCursor{ Windows::UI::Core::CoreCursorType::Arrow, 1 });
        Windows::UI::Xaml::Window::Current().CoreWindow().PointerPosition(swapChain_PointerPressedAt_ScreenSpace);
    }

    void MainPage::swapChainPanel_PointerMoved(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e)
    {
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
        }
    }



    /*
    winrt::fire_and_forget MainPage::AssetCtx_AddStaticCollider_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        auto lifetime = get_strong();

        auto dialog = winrt::make<CreateColliderDialog>();

        co_await dialog.ShowAsync(Windows::UI::Xaml::Controls::ContentDialogPlacement::Popup);

        auto result = dialog.SelectedBones();

        auto resultView = result.GetView();

        Collider c;
        c.localPosition = DirectX::XMFLOAT3{ 0.0f, 5.0f, 0.0f };
        c.localRotation = DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f };
        c.boxArgs = DirectX::XMFLOAT3{ 10.0f, 10.0f, 10.0f };
        c.type = ColliderType::BOX;
        c.boneReference = 8;

        Global::Model->colliders.push_back(c);
    }*/


    void MainPage::Pivot_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        uint32_t selectionIndex = mainPivot().SelectedIndex();

        switch(selectionIndex) {
            case 0: mainFrame().Navigate(xaml_typename<EggAssetEditor::GeometryPage>(), DataContext()); break;
            case 1: mainFrame().Navigate(xaml_typename<EggAssetEditor::MaterialsPage>(), DataContext()); break;
            case 2: mainFrame().Navigate(xaml_typename<EggAssetEditor::BonesPage>(), DataContext()); break;
            case 3: mainFrame().Navigate(xaml_typename<EggAssetEditor::AnimationsPage>(), DataContext()); break;
            case 4: mainFrame().Navigate(xaml_typename<EggAssetEditor::CollidersPage>(), DataContext()); break;
        }

    }

    fire_and_forget MainPage::AssetCtx_ImportFBX_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        auto lifetime = get_strong();

        Windows::Storage::Pickers::FileOpenPicker fileOpenPicker;
        fileOpenPicker.FileTypeFilter().ReplaceAll({ L".fbx" });
        fileOpenPicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);


        /**/
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

            if(Global::Manifest == nullptr) {
                Global::Manifest = std::make_unique<Manifest>();
            }

            Global::Manifest->base.file = winrt::to_string(file.Path());

            Geometry_ListView_SelectAll();
        }/**/
    }

    winrt::fire_and_forget MainPage::AssetCtx_ImportAnimation_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        auto lifetime = get_strong();

        if(Global::Manifest == nullptr) {
            return;
        }

        Windows::Storage::Pickers::FileOpenPicker fileOpenPicker;
        fileOpenPicker.FileTypeFilter().ReplaceAll({ L".fbx" });
        
        fileOpenPicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        /**/
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

                Manifest::Animation anim;
                anim.name = animationName;
                anim.editorPlaybackSpeed = 1.0f;
                anim.editorPlaybackLoop = true;
                anim.source.file = winrt::to_string(file.Path());
                anim.source.reference = "0";
                Global::Manifest->animations.push_back(std::move(anim));

                loadingScreenTitle().Text(L"Processed " + to_hstring(i + 1) +  L"/" + to_hstring(files.Size()) + L" file(s)");
            }
        }
        /**/

        loadingScreen().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
    }


    winrt::fire_and_forget MainPage::FileCtx_SaveManifest_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        auto lifetime = get_strong();

        if(Global::Manifest == nullptr || Global::Model == nullptr) {
            return;
        }
        /**/
        Windows::Storage::Pickers::FileSavePicker fileSavePicker;
        fileSavePicker.SuggestedFileName(L"manifest");
        fileSavePicker.FileTypeChoices().Insert(L"JSON", winrt::single_threaded_vector<hstring>({ L".json" }));
        fileSavePicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        Windows::Storage::StorageFile file = co_await fileSavePicker.PickSaveFileAsync();

        if(file != nullptr) {
            Windows::Storage::CachedFileManager::DeferUpdates(file);

            Global::Manifest->colliders.clear();
            Global::Manifest->colliders = Global::Model->colliders;
            co_await Windows::Storage::FileIO::WriteTextAsync(file, winrt::to_hstring(Global::Manifest->Store().dump()));

            Windows::Storage::Provider::FileUpdateStatus status = co_await Windows::Storage::CachedFileManager::CompleteUpdatesAsync(file);

            if(status == Windows::Storage::Provider::FileUpdateStatus::Complete) {
                OutputDebugStringW(L"Ok\r\n");
            }
        }
        /**/


    }

    fire_and_forget MainPage::FileCtx_LoadManifest_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        auto lifetime = get_strong();

        Windows::Storage::Pickers::FileOpenPicker fileOpenPicker;

        fileOpenPicker.FileTypeFilter().ReplaceAll({ L".json" });
        fileOpenPicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        loadingScreen().Visibility(Windows::UI::Xaml::Visibility::Visible);
        loadingScreenTitle().Text(L"Waiting for manifest...");

        /**/
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
                Global::Manifest = std::make_unique<Manifest>();
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

        }/**/

        loadingScreen().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
    }

    winrt::fire_and_forget MainPage::FileCtx_Compile_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        Windows::Storage::Pickers::FileSavePicker fileSavePicker;

        fileSavePicker.SuggestedFileName(L"asset");
        fileSavePicker.FileTypeChoices().Insert(L"EggAsset", winrt::single_threaded_vector<hstring>({ L".eggasset" }));
        fileSavePicker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Objects3D);

        /**/
        Windows::Storage::StorageFile file = co_await fileSavePicker.PickSaveFileAsync();
       // Windows::Storage::StorageFile & file = *(reinterpret_cast<Windows::Storage::StorageFile *>(0));

        if(file != nullptr) {
            Windows::Storage::CachedFileManager::DeferUpdates(file);

            //Windows::Storage::FileIO::WriteBytesAsync()
            
            //co_await Windows::Storage::FileIO::WriteTextAsync(file, ...);

            Windows::Storage::Provider::FileUpdateStatus status = co_await Windows::Storage::CachedFileManager::CompleteUpdatesAsync(file);

            if(status == Windows::Storage::Provider::FileUpdateStatus::Complete) {
                OutputDebugStringW(L"Egg Asset Saved\r\n");
            }
        }
        /**/
    }

}
