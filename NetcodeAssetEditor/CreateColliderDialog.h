#pragma once
#include "CreateColliderDialog.g.h"
#include "DC_Bone.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct CreateColliderDialog : CreateColliderDialogT<CreateColliderDialog>
    {
        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Bone> bones;
        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Bone> selectedBones;

        CreateColliderDialog();

        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Bone> Bones();
        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Bone> SelectedBones();

        void ContentDialog_PrimaryButtonClick(winrt::Windows::UI::Xaml::Controls::ContentDialog const & sender, winrt::Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const & args);
        void ContentDialog_SecondaryButtonClick(winrt::Windows::UI::Xaml::Controls::ContentDialog const & sender, winrt::Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const & args);
        void ContentDialog_Opened(winrt::Windows::UI::Xaml::Controls::ContentDialog const & sender, winrt::Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs const & args);
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct CreateColliderDialog : CreateColliderDialogT<CreateColliderDialog, implementation::CreateColliderDialog>
    {
    };
}
