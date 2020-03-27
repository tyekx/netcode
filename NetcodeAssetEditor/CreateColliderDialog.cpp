#include "pch.h"
#include "CreateColliderDialog.h"
#include "CreateColliderDialog.g.cpp"
#include "XamlGlobal.h"
#include "XamlHelpers.h"

namespace winrt::EggAssetEditor::implementation
{
	CreateColliderDialog::CreateColliderDialog() : CreateColliderDialogT<CreateColliderDialog>() {
		InitializeComponent();

		bones = winrt::single_threaded_observable_vector<EggAssetEditor::DC_Bone>();
		selectedBones = winrt::single_threaded_observable_vector<EggAssetEditor::DC_Bone>();
	}

    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> CreateColliderDialog::Bones()
    {
		return bones;
    }

	void CreateColliderDialog::ContentDialog_PrimaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog const & sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const & args)
	{
		auto deferral = args.GetDeferral();

		selectedBones.Clear();

		auto selectedItems = bonesList().SelectedItems();
		
		for(uint32_t i = 0; i < selectedItems.Size(); ++i) {
			selectedBones.Append(selectedItems.GetAt(i).as<EggAssetEditor::DC_Bone>());
		}

		deferral.Complete();
	}


	void CreateColliderDialog::ContentDialog_SecondaryButtonClick(Windows::UI::Xaml::Controls::ContentDialog const & sender, Windows::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const & args)
	{
		auto deferral = args.GetDeferral();

		selectedBones.Clear();
		
		deferral.Complete();
	}


	void CreateColliderDialog::ContentDialog_Opened(Windows::UI::Xaml::Controls::ContentDialog const & sender, Windows::UI::Xaml::Controls::ContentDialogOpenedEventArgs const & args)
	{
		if(Global::Model == nullptr || Global::Model->skeleton.bones.empty()) {
			return;
		}

		bones.Clear();

		auto dcBones = XamlHelpers::ConvertBones();
		auto dcBonesView = dcBones.GetView();

		for(uint32_t i = 0; i < dcBonesView.Size(); ++i) {

			bones.Append(dcBonesView.GetAt(i));
		}
	}

	Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> CreateColliderDialog::SelectedBones()
	{
		return selectedBones;
	}
}
