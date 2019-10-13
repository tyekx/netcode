#pragma once
#include "ProjectDataContext.g.h"

namespace winrt::EggEditor2::implementation
{
    struct ProjectDataContext : ProjectDataContextT<ProjectDataContext>
    {
	private:
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
		hstring name;
		hstring filePath;

	public:
        ProjectDataContext() = default;

        hstring Name();
        void Name(hstring const& value);

        hstring FilePath();
        void FilePath(hstring const& value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}

namespace winrt::EggEditor2::factory_implementation
{
	struct ProjectDataContext : ProjectDataContextT<ProjectDataContext, implementation::ProjectDataContext>
	{
	};
}
