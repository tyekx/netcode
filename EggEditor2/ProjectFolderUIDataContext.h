#pragma once
#include "ProjectFolderUIDataContext.g.h"

namespace winrt::EggEditor2::implementation
{
    struct ProjectFolderUIDataContext : ProjectFolderUIDataContextT<ProjectFolderUIDataContext>
    {
	private:
		int32_t editState;
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

	public:
        ProjectFolderUIDataContext() = default;

        int32_t EditState();
        void EditState(int32_t value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}

namespace winrt::EggEditor2::factory_implementation
{
	struct ProjectFolderUIDataContext : ProjectFolderUIDataContextT<ProjectFolderUIDataContext, implementation::ProjectFolderUIDataContext>
	{
	};
}
