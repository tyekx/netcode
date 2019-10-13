#pragma once
#include "AssetBrowserDataContext.g.h"
#include "ProjectFolderDataContext.h"

namespace winrt::EggEditor2::implementation
{
    struct AssetBrowserDataContext : AssetBrowserDataContextT<AssetBrowserDataContext>
    {
	private:
		ProjectFolderDataContext root;
		EggEditor2::IAsset currentFolder;
		Windows::Foundation::Collections::IObservableVector<EggEditor2::IAsset> breadCrumbs;
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
	public:
		AssetBrowserDataContext();

        EggEditor2::IAsset Root();
        EggEditor2::IAsset CurrentFolder();
        Windows::Foundation::Collections::IObservableVector<EggEditor2::IAsset> BreadCrumbs();

        void ChangeFolder(EggEditor2::IAsset const& folder);
        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::EggEditor2::factory_implementation
{
    struct AssetBrowserDataContext : AssetBrowserDataContextT<AssetBrowserDataContext, implementation::AssetBrowserDataContext>
    {
    };
}
