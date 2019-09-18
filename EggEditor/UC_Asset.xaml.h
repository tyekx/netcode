//
// UC_Asset.xaml.h
// Declaration of the UC_Asset class
//

#pragma once

#include "Auxiliary.h"
#include "UC_Asset.g.h"
#include "UC_ProjectFolder.xaml.h"

#define ASSET_TYPE_SHADED_MESH 0x0000
#define ASSET_TYPE_MATERIAL 0x0001
#define ASSET_TYPE_TEXTURE2D 0x0002
#define ASSET_TYPE_MESH 0x0003
#define ASSET_TYPE_SHADER 0x4000
#define ASSET_TYPE_VERTEX_SHADER 0x4001
#define ASSET_TYPE_GEOMETRY_SHADER 0x4002
#define ASSET_TYPE_DOMAIN_SHADER 0x4003
#define ASSET_TYPE_HULL_SHADER 0x4004
#define ASSET_TYPE_PIXEL_SHADER 0x4005
#define ASSET_TYPE_COMPUTE_SHADER 0x4006

namespace EggEditor
{

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

		event OpenAssetCallback ^ OpenAsset;

		property int AssetType {
			int get() {
				return assetType;
			}
		}

		property Platform::String ^ VirtualPath {
			Platform::String ^ get() {
				return ParentFolder->VirtualPath + TypedDataContext->AssetName;
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

		static unsigned int GetAssetTypeFromExtension(Platform::String ^ ext) {
			if(ext == L".jpg" || ext == L".png" || ext == L".jpeg") {
				return ASSET_TYPE_TEXTURE2D;
			} else if(ext == L".obj") {
				return ASSET_TYPE_MESH;
			} else if(ext == L".cso") {
				return ASSET_TYPE_SHADER;
			}
			return UINT_MAX;
		}

	private:
		void AssetDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args);
		void MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e);
		void CtxOpenInEditorBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
	};
}
