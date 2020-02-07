#include "pch.h"
#include "XamlHelpers.h"
#include "XamlHelpers.g.cpp"
#include "Model.h"
#include "XamlGlobal.h"
#include "DC_Bone.h"

namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> XamlHelpers::ConvertBones()
    {
        const auto & sk = Global::Model->skeleton;

        winrt::Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> bones =
            winrt::single_threaded_observable_vector<EggAssetEditor::DC_Bone>();

        for(const SkeletonBone & bone : sk.bones) {
            auto dcBone = winrt::make<DC_Bone>();
            dcBone.Name(winrt::to_hstring(bone.boneName));
            dcBone.Depth(0);

            uint32_t size = bones.Size();

            if(bone.parentIndex != -1) {
                auto b = bones.GetView();

                dcBone.Depth(b.GetAt(bone.parentIndex).Depth() + 1);
            }

            bones.Append(dcBone);
        }

        return bones;
    }
}
