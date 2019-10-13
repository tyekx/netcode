#include "pch.h"
#include "EditableTextBlockDataContext.h"
#include "EditableTextBlockDataContext.g.cpp"

namespace winrt::EggEditor2::implementation
{
    bool EditableTextBlockDataContext::IsEditing()
    {
		return isEditing;
    }
    void EditableTextBlockDataContext::IsEditing(bool value)
    {
		if(isEditing != value) {
			isEditing = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"IsEditing" });
		}
    }

	hstring EditableTextBlockDataContext::Text() {
		return textValue;
	}

	void EditableTextBlockDataContext::Text(hstring const & value) {
		if(textValue != value) {
			textValue = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"Text" });
		}
	}

    winrt::event_token EditableTextBlockDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return propertyChanged.add(handler);
    }
    void EditableTextBlockDataContext::PropertyChanged(winrt::event_token const& token) noexcept
    {
		propertyChanged.remove(token);
    }
}
