#include "pch.h"
#include "ProjectFolderDataContext.h"
#include "ProjectFolderDataContext.g.cpp"
#include "ProjectFolderUIDataContext.h"

namespace winrt::EggEditor2::implementation
{
	ProjectFolderDataContext::ProjectFolderDataContext() :
		folderId{ -1 },
		uiContext{ nullptr },
		children{ },
		propertyChanged{},
		name{},
		filePath{},
		parentFolder{ nullptr },
		isDirty{ false } {
		children = winrt::single_threaded_observable_vector<IAsset>();
	}

    hstring ProjectFolderDataContext::FilePath()
    {
		return filePath;
    }
    void ProjectFolderDataContext::FilePath(hstring const& value)
    {
		if(filePath != value) {
			filePath = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"FilePath" });
		}
    }
    int32_t ProjectFolderDataContext::FolderId()
    {
		return folderId;
    }
    void ProjectFolderDataContext::FolderId(int32_t value)
    {
		if(folderId != value) {
			folderId = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"FolderId" });
		}
    }
    EggEditor2::ProjectFolderUIDataContext ProjectFolderDataContext::UI()
    {
		return uiContext;
    }
    void ProjectFolderDataContext::InsertFolder(EggEditor2::IAsset const& asset)
    {
		children.Append(asset);
    }
    void ProjectFolderDataContext::InsertAsset(EggEditor2::IAsset const& asset)
    {
		children.Append(asset);
    }
    winrt::event_token ProjectFolderDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return propertyChanged.add(handler);
    }
    void ProjectFolderDataContext::PropertyChanged(winrt::event_token const& token) noexcept
    {
		propertyChanged.remove(token);
    }
    hstring ProjectFolderDataContext::Name()
    {
		return name;
    }
    void ProjectFolderDataContext::Name(hstring const& value)
    {
		if(name != value) {
			name = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"Name" });
		}
    }
    EggEditor2::IAsset ProjectFolderDataContext::ParentFolder()
    {
		return parentFolder;
    }
    void ProjectFolderDataContext::ParentFolder(EggEditor2::IAsset const& value)
    {
		if(parentFolder != value) {
			parentFolder = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"ParentFolder" });
		}
    }
    bool ProjectFolderDataContext::IsDirty()
    {
		return isDirty;
    }
    void ProjectFolderDataContext::IsDirty(bool value)
    {
		if(isDirty != value) {
			isDirty = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"IsDirty" });
		}
    }
    bool ProjectFolderDataContext::IsAsset()
    {
		return false;
    }
    bool ProjectFolderDataContext::IsContentDirty()
    {
		return IsDirty();
    }
    Windows::Foundation::Collections::IObservableVector<EggEditor2::IAsset> ProjectFolderDataContext::Children()
    {
		return children;
    }
    void ProjectFolderDataContext::AddChild(EggEditor2::IAsset const& asset)
    {
		children.Append(asset);
		/*
		if(asset.IsAsset()) {
			InsertAsset(asset);
		} else {
			InsertFolder(asset);
		}*/
    }
}
