//
// UC_Asset.xaml.h
// Declaration of the UC_Asset class
//

#pragma once

#include "Auxiliary.h"
#include "UC_Asset.g.h"
#include "UC_ProjectFolder.xaml.h"


namespace EggEditor
{
#define ASSET_TYPE_SHADED_MESH 0
#define ASSET_TYPE_MATERIAL 1

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_Asset sealed
	{
		UC_ProjectFolder ^ parentFolder;
		unsigned int assetType;

		void ImplDetail_InsertAssetIntoParent();

		static UC_Asset ^ currentlyDragging;
	public:
		UC_Asset(Platform::String^ text, UINT atype);

		static property UC_Asset ^ CurrentlyDragging {
			UC_Asset ^ get() {
				return currentlyDragging;
			}
		}

		property UC_ProjectFolder ^ ParentFolder {
			UC_ProjectFolder ^ get() {
				return parentFolder;
			}
			void set(UC_ProjectFolder ^ val) {
				if(parentFolder) {
					unsigned int id;
					if(parentFolder->Children->IndexOf(this, &id)) {
						parentFolder->Children->RemoveAt(id);
					}
				}
				parentFolder = val;
				ImplDetail_InsertAssetIntoParent();
			}
		}
		
		property AssetDataContext ^ TypedDataContext {
			AssetDataContext ^ get() {
				return (AssetDataContext ^)DataContext;
			}
		}

	private:
		void AssetDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args);
		void MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e);
	};
}
