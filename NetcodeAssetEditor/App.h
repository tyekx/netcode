#pragma once
#include "App.xaml.g.h"
#include "MainPage.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const &);
        void OnSuspending(IInspectable const &, Windows::ApplicationModel::SuspendingEventArgs const &);
        void OnNavigationFailed(IInspectable const &, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const &);
    };
}
