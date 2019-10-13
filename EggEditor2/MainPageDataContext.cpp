#include "pch.h"
#include "MainPageDataContext.h"
#include "MainPageDataContext.g.cpp"

namespace winrt::EggEditor2::implementation
{
    EggEditor2::ProjectDataContext MainPageDataContext::Project()
    {
		return project;
    }
    void MainPageDataContext::Project(EggEditor2::ProjectDataContext const& value)
    {
		project = value;
		propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"Project" });
    }
    int32_t MainPageDataContext::MainWindowState()
    {
		return mainWindowState;
    }
    void MainPageDataContext::MainWindowState(int32_t value)
    {
		if(mainWindowState != value) {
			mainWindowState = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"MainWindowState" });
		}
    }
    int32_t MainPageDataContext::SubWindowState()
    {
		return subWindowState;
    }
    void MainPageDataContext::SubWindowState(int32_t value)
    {
		if(subWindowState != value) {
			subWindowState = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"SubWindowState" });
		}
    }
    winrt::event_token MainPageDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return propertyChanged.add(handler);
    }
    void MainPageDataContext::PropertyChanged(winrt::event_token const& token) noexcept
    {
		propertyChanged.remove(token);
    }
}
