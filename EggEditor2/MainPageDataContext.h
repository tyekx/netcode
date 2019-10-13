#pragma once
#include "MainPageDataContext.g.h"

namespace winrt::EggEditor2::implementation
{
    struct MainPageDataContext : MainPageDataContextT<MainPageDataContext>
    {
	private:
		int32_t mainWindowState;
		int32_t subWindowState;
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
		EggEditor2::ProjectDataContext project;

	public:


        MainPageDataContext() = default;

        EggEditor2::ProjectDataContext Project();
        void Project(EggEditor2::ProjectDataContext const& value);
        int32_t MainWindowState();
        void MainWindowState(int32_t value);
        int32_t SubWindowState();
        void SubWindowState(int32_t value);
        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::EggEditor2::factory_implementation
{
    struct MainPageDataContext : MainPageDataContextT<MainPageDataContext, implementation::MainPageDataContext>
    {
    };
}
