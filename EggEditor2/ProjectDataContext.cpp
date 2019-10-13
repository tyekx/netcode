#include "pch.h"
#include "ProjectDataContext.h"
#include "ProjectDataContext.g.cpp"

namespace winrt::EggEditor2::implementation
{
    hstring ProjectDataContext::Name()
    {
		return name;
    }

    void ProjectDataContext::Name(hstring const& value)
    {
		if(name != value) {
			name = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"Name" });
		}
    }

    hstring ProjectDataContext::FilePath()
    {
		return filePath;
    }

    void ProjectDataContext::FilePath(hstring const& value)
    {
		if(filePath != value) {
			filePath = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"FilePath" });
		}
    }

    winrt::event_token ProjectDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return propertyChanged.add(handler);
    }

    void ProjectDataContext::PropertyChanged(winrt::event_token const& token) noexcept
    {
		propertyChanged.remove(token);
    }
}
