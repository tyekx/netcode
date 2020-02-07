#pragma once

#include "DC_Collider.h"
#include "CollidersPage.g.h"
#include "ColliderTypeVisibilityConverter.h"
#include "VectorConverter.h"

namespace winrt::EggAssetEditor::implementation
{
    struct CollidersPage : CollidersPageT<CollidersPage>
    {
        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> colliders;

        CollidersPage();

        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> Colliders();

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
    };
}

namespace winrt::EggAssetEditor::factory_implementation
{
    struct CollidersPage : CollidersPageT<CollidersPage, implementation::CollidersPage>
    {
    };
}
