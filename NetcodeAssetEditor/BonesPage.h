#pragma once

#include "BonesPage.g.h"
#include "DepthToMarginConverter.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct BonesPage : BonesPageT<BonesPage>
    {
        bool firstNavigation;

        BonesPage();
        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
        void bonesList_SelectionChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e);
    };
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct BonesPage : BonesPageT<BonesPage, implementation::BonesPage>
    {
    };
}
