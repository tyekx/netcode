#pragma once
#include "ProjectFolderDataContext.g.h"
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::EggEditor2::implementation
{
    struct ProjectFolderDataContext : ProjectFolderDataContextT<ProjectFolderDataContext>
    {
	private:
		int32_t folderId;
		EggEditor2::ProjectFolderUIDataContext uiContext;
		Windows::Foundation::Collections::IObservableVector<IAsset> children;
		winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
		hstring name;
		hstring filePath;
		IAsset parentFolder;
		bool isDirty;
	public:

		ProjectFolderDataContext();

        hstring FilePath();
        void FilePath(hstring const& value);

        int32_t FolderId();
        void FolderId(int32_t value);

        EggEditor2::ProjectFolderUIDataContext UI();
        void InsertFolder(EggEditor2::IAsset const& asset);
        void InsertAsset(EggEditor2::IAsset const& asset);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        hstring Name();
        void Name(hstring const& value);

        EggEditor2::IAsset ParentFolder();
        void ParentFolder(EggEditor2::IAsset const& value);
        bool IsDirty();
        void IsDirty(bool value);
        bool IsAsset();
        bool IsContentDirty();
        Windows::Foundation::Collections::IObservableVector<EggEditor2::IAsset> Children();
        void AddChild(EggEditor2::IAsset const& asset);
    };
}

namespace winrt::EggEditor2::factory_implementation
{
	struct ProjectFolderDataContext : ProjectFolderDataContextT<ProjectFolderDataContext, implementation::ProjectFolderDataContext>
	{
	};
}
