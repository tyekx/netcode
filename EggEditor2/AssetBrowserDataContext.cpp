#include "pch.h"
#include "AssetBrowserDataContext.h"
#include "AssetBrowserDataContext.g.cpp"
#include "AssetDataContext.h"


namespace winrt::EggEditor2::implementation
{

	AssetBrowserDataContext::AssetBrowserDataContext() : root{}, currentFolder{}, breadCrumbs{ winrt::single_threaded_observable_vector<IAsset>() }, propertyChanged{} {
		root.Name(L"/");

		EggEditor2::IAsset pfdc1 = winrt::make<EggEditor2::implementation::AssetDataContext>( ASSET_TYPE_MATERIAL );
		pfdc1.Name(L"TestMaterial");
		
		root.AddChild(pfdc1);
		currentFolder = root;
	}

    EggEditor2::IAsset AssetBrowserDataContext::Root()
    {
		return root;
    }
    EggEditor2::IAsset AssetBrowserDataContext::CurrentFolder()
    {
		return currentFolder;
    }
    Windows::Foundation::Collections::IObservableVector<EggEditor2::IAsset> AssetBrowserDataContext::BreadCrumbs()
    {
		return breadCrumbs;
    }
    void AssetBrowserDataContext::ChangeFolder(EggEditor2::IAsset const& folder)
    {
		currentFolder = folder;
		propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"CurrentFolder" });
    }
    winrt::event_token AssetBrowserDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return propertyChanged.add(handler);
    }
    void AssetBrowserDataContext::PropertyChanged(winrt::event_token const& token) noexcept
    {
		propertyChanged.remove(token);
    }
}
