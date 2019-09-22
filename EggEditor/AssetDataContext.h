#pragma once

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

namespace EggEditor {

	[Windows::UI::Xaml::Data::Bindable]
	public ref class AssetUIDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		Windows::UI::Color gradientStart;
		Windows::UI::Color gradientStop;
		Platform::String ^ overlayText;
		Windows::UI::Xaml::Controls::Symbol assetIcon;
		int editState;

	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		AssetUIDataContext() : gradientStart{ 255,255,255,255 }, gradientStop{ 0,0,0,0 }, overlayText{ L"" }, editState{ 0 }, assetIcon{ Windows::UI::Xaml::Controls::Symbol::Help } {}

		property Windows::UI::Xaml::Controls::Symbol AssetIcon {
			Windows::UI::Xaml::Controls::Symbol get() {
				return assetIcon;
			}
			void set(Windows::UI::Xaml::Controls::Symbol sym) {
				assetIcon = sym;
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"AssetIcon"));
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
	public ref class AssetDataContext sealed : public Windows::UI::Xaml::Data::INotifyPropertyChanged {
		Platform::String ^ assetName;
		int assetType;
		AssetUIDataContext ^ uiContext;

		void SetAssetType(int value);

	public:
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

		AssetDataContext() : assetName{ L"" } {
			uiContext = ref new AssetUIDataContext();
		}

		property AssetUIDataContext ^ UI {
			AssetUIDataContext ^ get() {
				return uiContext;
			}
		}

		property int AssetType {
			int get() {
				return assetType;
			}
			void set(int value) {
				assetType = value;
				SetAssetType(assetType);
				PropertyChanged(this, ref new Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"AssetType"));
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
	};

}
