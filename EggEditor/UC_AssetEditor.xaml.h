//
// UC_AssetEditor.xaml.h
// Declaration of the UC_AssetEditor class
//

#pragma once

#include "UC_AssetEditor.g.h"
#include "UC_Asset.xaml.h"
#include "Auxiliary.h"

namespace EggEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_AssetEditor sealed
	{
		UC_Asset ^ currentlyEditing;
		AssetEditorDataContext ^ TypedDataContext;
	public:

		void SetAsset(UC_Asset ^ asset) {
			currentlyEditing = asset;
			TypedDataContext->AssetType = asset->TypedDataContext->AssetType;
		}

		UC_AssetEditor();
	};
}
