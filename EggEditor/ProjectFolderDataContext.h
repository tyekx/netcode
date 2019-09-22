#pragma once

#include "AssetDataContext.h"
#include <collection.h>
#include "IAssetBrowserItem.h"

namespace EggEditor {

	[Windows::UI::Xaml::Data::Bindable]
	public ref class ProjectFolderUIDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		int editState;
	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		ProjectFolderUIDataContext() : editState{ 0 } { }

		property int EditState {
			int get() {
				return editState;
			}
			void set(int val) {
				editState = val;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"EditState"));
			}
		}
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class ProjectFolderDataContext sealed : public IAssetBrowserItem, public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		int folderId;
		Platform::String ^ folderName;
		ProjectFolderDataContext ^ parentFolder;
		ProjectFolderUIDataContext ^ uiContext;
		Platform::Collections::Vector<IAssetBrowserItem ^> ^ children;
		bool isDirty;
	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		ProjectFolderDataContext() {
			uiContext = ref new ProjectFolderUIDataContext();
			children = ref new Platform::Collections::Vector<IAssetBrowserItem ^>();
		}

		property  ProjectFolderUIDataContext ^ UI {
			ProjectFolderUIDataContext ^ get() {
				return uiContext;
			}
		}

		virtual void AddChild(IAssetBrowserItem ^ item) {
			if(item->IsAsset) {
				InsertAsset(item);
			} else {
				InsertFolder(item);
			}
		}

		virtual property bool IsContentDirty {
			bool get() {
				if(IsDirty) {
					return true;
				}
				for(IAssetBrowserItem ^ i : children) {
					if(i->IsContentDirty) {
						return true;
					}
				}
				return false;
			}
		}

		virtual property bool IsDirty {
			bool get() {
				return isDirty;
			}
			void set(bool v) {
				v = isDirty;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"IsDirty"));
			}
		}

		void InsertAsset(IAssetBrowserItem ^ asset) {
			for(unsigned int i = 0; i < children->Size; ++i) {
				IAssetBrowserItem ^ element = children->GetAt(i);

				if(element->IsAsset) {
					if(std::wcscmp(element->Name->Data(), asset->Name->Data()) > 0) {
						children->InsertAt(i, asset);
						return;
					}
				}
			}

			children->Append(asset);
		}

		void InsertFolder(IAssetBrowserItem ^ folder) {
			for(unsigned int i = 0; i < children->Size; ++i) {
				IAssetBrowserItem ^ element = children->GetAt(i);

				// if we found a file, then insert it before the file
				if(element->IsAsset) {
					children->InsertAt(i, folder);
					return;
				}

				if(std::wcscmp(element->Name->Data(), folder->Name->Data()) > 0) {
					children->InsertAt(i, folder);
					return;
				}
			}

			// append if lookup failed
			children->Append(folder);
		}

		virtual property bool IsAsset {
			bool get() {
				return false;
			}
		}

		property Platform::String ^ VirtualPath {
			Platform::String ^ get() {
				if(parentFolder == nullptr) {
					return L"/";
				}
				return parentFolder->VirtualPath + Name + L"/";
			}
		}

		virtual property Windows::Foundation::Collections::IObservableVector<IAssetBrowserItem ^> ^ Children {
			Windows::Foundation::Collections::IObservableVector<IAssetBrowserItem ^> ^ get() {
				return children;
			}
		}

		virtual property IAssetBrowserItem ^ ParentFolder {
			IAssetBrowserItem ^ get() {
				return parentFolder;
			}
			void set(IAssetBrowserItem ^ pf) {
				if(pf->IsAsset) {
					return;
				}
				if(parentFolder) {
					unsigned int id;
					if(parentFolder->Children->IndexOf(this, &id)) {
						parentFolder->Children->RemoveAt(id);
					}
				}
				parentFolder = (ProjectFolderDataContext^)pf;
				parentFolder->InsertFolder(this);
			}
		}

		virtual property Platform::String ^ Name {
			Platform::String ^ get() {
				return folderName;
			}
			void set(Platform::String ^ str) {
				folderName = str;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Name"));
			}
		}

		property int FolderId {
			int get() {
				return folderId;
			}
			void set(int val) {
				folderId = val;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"FolderId"));
			}
		}
	};

}
