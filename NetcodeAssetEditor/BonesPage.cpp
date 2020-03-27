#include "pch.h"
#include "BonesPage.h"
#if __has_include("BonesPage.g.cpp")
#include "BonesPage.g.cpp"
#endif

#include "DC_Bone.h"
#include "XamlGlobal.h"
#include "XamlHelpers.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NetcodeAssetEditor::implementation
{
    BonesPage::BonesPage()
    {
        InitializeComponent();

        firstNavigation = true;
    }

    void BonesPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        if(firstNavigation) {
            DataContext(e.Parameter());

            firstNavigation = false;
        }

        if(Global::Model != nullptr && !Global::Model->meshes.empty()) {
            Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Bone> bones = XamlHelpers::ConvertBones();

            bonesList().ItemsSource(bones);
        }

        __super::OnNavigatedTo(e);
    }

    void BonesPage::bonesList_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        uint32_t selectedItemsCount = bonesList().SelectedItems().Size();

        if(Global::Model == nullptr) {
            return;
        }

        if(selectedItemsCount == 0) {
            Global::EditorApp->SetSelectedBones({});
            return;
        }

        auto selectedRanges = bonesList().SelectedRanges();

        std::vector<uint32_t> selectedBoneIndices;
        selectedBoneIndices.reserve(selectedItemsCount);

        for(const auto range : selectedRanges) {
            int32_t firstIndex = range.FirstIndex();
            int32_t lastIndex = range.LastIndex();

            for(int32_t i = firstIndex; i <= lastIndex; ++i) {
                selectedBoneIndices.push_back(static_cast<uint32_t>(i));
            }
        }

        Global::EditorApp->SetSelectedBones(selectedBoneIndices);
        Global::EditorApp->Run();
    }
}



