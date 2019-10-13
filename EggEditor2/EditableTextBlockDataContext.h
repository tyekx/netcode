#pragma once
#include "EditableTextBlockDataContext.g.h"

namespace winrt::EggEditor2::implementation
{
    struct EditableTextBlockDataContext : EditableTextBlockDataContextT<EditableTextBlockDataContext>
    {
	private:
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
		hstring textValue;
		bool isEditing;
	public:

        EditableTextBlockDataContext() = default;

		hstring Text();
		void Text(hstring const & value);
        bool IsEditing();
        void IsEditing(bool value);
        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::EggEditor2::factory_implementation
{
    struct EditableTextBlockDataContext : EditableTextBlockDataContextT<EditableTextBlockDataContext, implementation::EditableTextBlockDataContext>
    {
    };
}
