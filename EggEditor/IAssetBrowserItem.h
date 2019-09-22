#pragma once

namespace EggEditor {

	public interface class IAssetBrowserItem {
	public:
		/* getters, setters */
		property Platform::String ^ Name;
		property IAssetBrowserItem ^ ParentFolder;
		property bool IsDirty;

		/* getters */
		property bool IsAsset {
			bool get();
		}

		/*
		* A query if any sub-asset is dirty
		*/
		property bool IsContentDirty {
			bool get();
		}

		property Windows::Foundation::Collections::IObservableVector<IAssetBrowserItem ^> ^ Children {
			Windows::Foundation::Collections::IObservableVector<IAssetBrowserItem ^> ^ get();
		}

		virtual void AddChild(IAssetBrowserItem ^ item);
	};

}
