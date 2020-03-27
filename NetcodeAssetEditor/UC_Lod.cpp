#include "pch.h"
#include "UC_Lod.h"
#include "UC_Lod.g.cpp"

namespace winrt::NetcodeAssetEditor::implementation
{
    UC_Lod::UC_Lod() {
        InitializeComponent();
        MyContent(winrt::make<DC_Lod>());
    }

    UC_Lod::UC_Lod(NetcodeAssetEditor::DC_Lod const & myContent) {
        InitializeComponent();
        MyContent(myContent);
    }

    NetcodeAssetEditor::DC_Lod UC_Lod::MyContent()
    {
        return DataContext().as<NetcodeAssetEditor::DC_Lod>();
    }

    void UC_Lod::MyContent(NetcodeAssetEditor::DC_Lod const & value)
    {
        DataContext(value);
    }
}
