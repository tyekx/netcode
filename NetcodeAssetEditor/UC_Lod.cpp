#include "pch.h"
#include "UC_Lod.h"
#include "UC_Lod.g.cpp"

namespace winrt::EggAssetEditor::implementation
{
    UC_Lod::UC_Lod() {
        InitializeComponent();
        MyContent(winrt::make<DC_Lod>());
    }

    UC_Lod::UC_Lod(EggAssetEditor::DC_Lod const & myContent) {
        InitializeComponent();
        MyContent(myContent);
    }

    EggAssetEditor::DC_Lod UC_Lod::MyContent()
    {
        return DataContext().as<EggAssetEditor::DC_Lod>();
    }

    void UC_Lod::MyContent(EggAssetEditor::DC_Lod const & value)
    {
        DataContext(value);
    }
}
