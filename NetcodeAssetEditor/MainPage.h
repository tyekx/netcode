#pragma once
#include "XamlHelpers.h"
#include "DC_Animation.h"
#include "DC_Lod.h"
#include "DC_Bone.h"
#include "UC_Mesh.h"
#include "DC_Mesh.h"
#include "DC_MainPage.h"
#include "DC_Material.h"
#include "MainPage.g.h"
#include <Netcode/Modules.h>
#include "XamlGlobal.h"
#include "MaterialsPage.h"
#include "CollidersPage.h"
#include "ColliderTypeConverter.h"
#include "ColliderTypeVisibilityConverter.h"
#include "SelectionToVisibilityConverter.h"
#include "DepthToMarginConverter.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        Windows::Foundation::Point swapChain_PointerPressedAt_ScreenSpace;
        Windows::Foundation::Point swapChain_LastPointerPosition_PanelSpace;

        winrt::event<NetcodeAssetEditor::ManifestChangedHandler> manifestChanged;
        winrt::event<NetcodeAssetEditor::ModelChangedHandler> modelChanged;

        bool pointerHeld;

        MainPage();

        winrt::event_token ManifestChanged(NetcodeAssetEditor::ManifestChangedHandler const & handler);
        void ManifestChanged(winrt::event_token const & token) noexcept;

        winrt::event_token ModelChanged(NetcodeAssetEditor::ModelChangedHandler const & handler);
        void ModelChanged(winrt::event_token const & token) noexcept;

		void FileCtx_CreateManifest_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        winrt::fire_and_forget FileCtx_SaveManifest_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        winrt::fire_and_forget FileCtx_Compile_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        winrt::fire_and_forget FileCtx_LoadManifest_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e);

        winrt::fire_and_forget AssetCtx_ImportFBX_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        winrt::fire_and_forget AssetCtx_ImportAnimation_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        

    

        void swapChainPanel_Loaded(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        void swapChainPanel_PointerPressed(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e);
        void swapChainPanel_PointerReleased(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e);
        void swapChainPanel_PointerMoved(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e);

        void Geometry_ListView_SelectAll();
        void Pivot_SelectionChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e);
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
