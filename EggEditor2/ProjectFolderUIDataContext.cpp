#include "pch.h"
#include "ProjectFolderUIDataContext.h"
#include "ProjectFolderUIDataContext.g.cpp"

namespace winrt::EggEditor2::implementation
{
    int32_t ProjectFolderUIDataContext::EditState()
    {
		return editState;
    }

    void ProjectFolderUIDataContext::EditState(int32_t value)
    {
		if(editState != value) {
			editState = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"EditState" });
		}
    }

    winrt::event_token ProjectFolderUIDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return propertyChanged.add(handler);
    }

    void ProjectFolderUIDataContext::PropertyChanged(winrt::event_token const& token) noexcept
    {
		propertyChanged.remove(token);
    }
}
