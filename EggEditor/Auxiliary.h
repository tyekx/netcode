#pragma once

#include <Egg/GameObjectObserver.h>
#include "Adapters.h"

namespace EggEditor {


	public delegate void OpenAssetCallback(Platform::Object ^ asset);
	/*
	Converts to Boolean
	*/
	public ref class IntNotEqualConverter sealed : public Windows::UI::Xaml::Data::IValueConverter {
	public:

		// Inherited via IValueConverter
		virtual Platform::Object ^ Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
		virtual Platform::Object ^ ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
	};

	/*
	Converts To Visibility
	*/
	public ref class IntEqualConverter sealed : public Windows::UI::Xaml::Data::IValueConverter {
	public:

		// Inherited via IValueConverter
		virtual Platform::Object ^ Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
		virtual Platform::Object ^ ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
	};

	/*
	* Converts To Visibility
	*/
	public ref class SignatureToVisibilityConverter sealed : public Windows::UI::Xaml::Data::IValueConverter {
	public:
		virtual Platform::Object ^ Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
		virtual Platform::Object ^ ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
	};
	/*
	* Converts To Visibility
	*/
	public ref class NegatedSignatureToVisibilityConverter sealed : public Windows::UI::Xaml::Data::IValueConverter {
	public:
		virtual Platform::Object ^ Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
		virtual Platform::Object ^ ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
	};

	/*
	* Visible if there more components to add in the signature
	*/
	public ref class SignatureBitsToVisibilityConverter sealed : public Windows::UI::Xaml::Data::IValueConverter {
	public:
		virtual Platform::Object ^ Convert(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
		virtual Platform::Object ^ ConvertBack(Platform::Object ^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^ parameter, Platform::String ^ language);
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class ProjectFolderDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		Platform::String ^ folderName;
		int editState;
	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		property Platform::String ^ FolderName {
			Platform::String ^ get() {
				return folderName;
			}
			void set(Platform::String ^ str) {
				folderName = str;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"FolderName"));
			}
		}

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
	public ref class BreadCrumbDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		Platform::String ^ text;
	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		property Platform::String ^ BreadCrumbText {
			Platform::String ^ get() {
				return text;
			}
			void set(Platform::String ^ str) {
				text = str;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"BreadCrumbText"));
			}
		}
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class AssetEditorDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		int assetType;
	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		property int AssetType {
			int get() {
				return assetType;
			}
			void set(int v) {
				assetType = v;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"AssetType"));
			}
		}
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class GameObjectDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		Platform::String ^ gameObjectName;
		SignatureType gameObjectSignature;
		CxCppGameObjectObserverAdapter gameObjectObserver;
		int editState;

	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		property Platform::String ^ Name {
			Platform::String ^ get() {
				return gameObjectName;
			}
			void set(Platform::String ^ str) {
				gameObjectName = str;
				/* insert feedback loop here */
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Name"));
			}
		}

		property SignatureType Signature {
			SignatureType get() {
				return gameObjectSignature;
			}
			void set(SignatureType signature) {
				gameObjectSignature = signature;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Signature"));
			}
		}

		property int EditState {
			int get() {
				return editState;
			}
			void set(int state) {
				editState = state;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"EditState"));
			}
		}

		void OnGameObjectNameChanged(UINT_PTR gameObject) {
			Egg::GameObject * obj = reinterpret_cast<Egg::GameObject *>(gameObject);
			// lot of allocation but it should be a fairly rare event
			std::wstring wName = Egg::Utility::ToWideString(obj->GetName());
			gameObjectName = ref new Platform::String(wName.c_str());
			PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Name"));
		}

		void OnGameObjectOwnerChanged(UINT_PTR gameObject) {

		}

		void OnGameObjectSignatureChanged(UINT_PTR gameObject) {
			Egg::GameObject * obj = reinterpret_cast<Egg::GameObject *>(gameObject);
			Signature = obj->GetSignature();
		}

		void OnGameObjectDestroyed(UINT_PTR gameObject) {

		}

		void SetGameObject(UINT_PTR gameObject) {
			reinterpret_cast<Egg::GameObject *>(gameObject)->SetObserver(&gameObjectObserver);
		}

		GameObjectDataContext() : gameObjectName{ L"" }, gameObjectSignature{ 0 }, gameObjectObserver{}, editState{ 0 } {
			gameObjectObserver.GetCx()->GameObjectNameChanged += ref new GameObjectChangedCallback(this, &GameObjectDataContext::OnGameObjectNameChanged);
			gameObjectObserver.GetCx()->GameObjectDestroyed += ref new GameObjectChangedCallback(this, &GameObjectDataContext::OnGameObjectDestroyed);
			gameObjectObserver.GetCx()->GameObjectOwnerChanged += ref new GameObjectChangedCallback(this, &GameObjectDataContext::OnGameObjectOwnerChanged);
			gameObjectObserver.GetCx()->GameObjectSignatureChanged += ref new GameObjectChangedCallback(this, &GameObjectDataContext::OnGameObjectSignatureChanged);
		}

	};


	[Windows::UI::Xaml::Data::Bindable]
	public ref class SceneDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		Platform::Collections::Vector<Windows::UI::Xaml::UIElement ^> ^ gameObjects;
	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		property Windows::Foundation::Collections::IVector<Windows::UI::Xaml::UIElement ^> ^ GameObjects {
			Windows::Foundation::Collections::IVector<Windows::UI::Xaml::UIElement ^> ^ get() {
				return gameObjects;
			}
		}

		SceneDataContext();
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class MainPageDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		int mainWindowIndex;
		int subWindowIndex;

	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;
		
		property int MainWindowIndex {
			int get() {
				return mainWindowIndex;
			}
			void set(int v) {
				mainWindowIndex = v;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"MainWindowIndex"));
			}
		}

		property int SubWindowIndex {
			int get() {
				return subWindowIndex;
			}
			void set(int v) {
				subWindowIndex = v;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"SubWindowIndex"));
			}
		}

	};
}
