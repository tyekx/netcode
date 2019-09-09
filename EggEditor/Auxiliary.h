#pragma once

#include "EggProject.h"
#include <Egg/GameObjectObserver.h>
#include "Adapters.h"

namespace EggEditor {

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
	public ref class AssetDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		Platform::String ^ assetName;
		Windows::UI::Color gradientStart;
		Windows::UI::Color gradientStop;
		Platform::String ^ overlayText;
		int editState;

	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		AssetDataContext() : assetName{ L"" }, gradientStart{ 255,255,255,255 }, gradientStop{ 0,0,0,0 }, overlayText{ L"" }, editState{ 0 } {}


		property int EditState {
			int get() {
				return editState;
			}
			void set(int val) {
				editState = val;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"EditState"));
			}
		}

		property Platform::String ^ AssetName {
			Platform::String ^ get() {
				return assetName;
			}
			void set(Platform::String ^ str) {
				assetName = str;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"AssetName"));
			}
		}

		property Platform::String ^ OverlayText {
			Platform::String ^ get() {
				return overlayText;
			}
			void set(Platform::String ^ str) {
				overlayText = str;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"OverlayText"));
			}
		}

		property Windows::UI::Color GradientStart {
			Windows::UI::Color get() {
				return gradientStart;
			}
			void set(Windows::UI::Color c) {
				gradientStart = c;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"GradientStart"));
			}
		}

		property Windows::UI::Color GradientStop {
			Windows::UI::Color get() {
				return gradientStop;
			}
			void set(Windows::UI::Color c) {
				gradientStop = c;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"GradientStop"));
			}
		}
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
