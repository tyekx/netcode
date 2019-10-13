#pragma once
#include "AssetDataContext.g.h"

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


namespace winrt::EggEditor2::implementation
{
    struct AssetDataContext : AssetDataContextT<AssetDataContext>
    {
	private:
		int32_t type;
		hstring name;
		EggEditor2::AssetUIDataContext uiContext;
		EggEditor2::IAsset parentFolder;
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
		bool isDirty;

		void SetAssetType(int type);

	public:
		AssetDataContext(int32_t assetType);
        AssetDataContext() = default;

        int32_t Type();
        void Type(int32_t value);

        EggEditor2::AssetUIDataContext UI();

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

		winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const & handler);
		void PropertyChanged(winrt::event_token const & token) noexcept;
    };
}

namespace winrt::EggEditor2::factory_implementation
{
	struct AssetDataContext : AssetDataContextT<AssetDataContext, implementation::AssetDataContext>
	{
	};
}
