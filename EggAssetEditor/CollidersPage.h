#pragma once

#include "DC_Collider.h"
#include "CollidersPage.g.h"
#include "ColliderTypeVisibilityConverter.h"
#include "VectorConverter.h"
#include "Vector3Control.h"
#include "Vector2Control.h"

namespace winrt::EggAssetEditor::implementation
{
    struct CollidersPage : CollidersPageT<CollidersPage>
    {
        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> colliders;
        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> bones;
        Windows::Foundation::Collections::IVector<winrt::event_token> tokens;

        winrt::event_token AddCallbackFor(uint32_t idx);
        EggAssetEditor::DC_Collider ConvertCollider(const Collider & collider);

        CollidersPage();

        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> Colliders();
        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> Bones();

        void OnCollidersCollectionChanged(
            Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> const & sender,
            Windows::Foundation::Collections::IVectorChangedEventArgs const & event);

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
        void ClearBoneReference_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        void AddCollider_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
		void collidersList_SelectionChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e);
        void boneComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e);
    };
}

namespace winrt::EggAssetEditor::factory_implementation
{
    struct CollidersPage : CollidersPageT<CollidersPage, implementation::CollidersPage>
    {
    };
}
