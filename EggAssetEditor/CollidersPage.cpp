#include "pch.h"
#include "CollidersPage.h"
#if __has_include("CollidersPage.g.cpp")
#include "CollidersPage.g.cpp"
#endif
#include "XamlGlobal.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggAssetEditor::implementation
{
    CollidersPage::CollidersPage()
    {
        InitializeComponent();

        colliders = winrt::single_threaded_observable_vector<EggAssetEditor::DC_Collider>();
    }

    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Collider> CollidersPage::Colliders() {
        return colliders;
    }

    void CollidersPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        colliders.Clear();

        if(Global::Model != nullptr && !Global::Model->colliders.empty()) {
            for(const auto & collider : Global::Model->colliders) {
                auto dcColliders = winrt::make<EggAssetEditor::implementation::DC_Collider>();
                dcColliders.LocalPosition(Windows::Foundation::Numerics::float3( collider.localPosition.x, collider.localPosition.y, collider.localPosition.z ));
                dcColliders.LocalRotation(Windows::Foundation::Numerics::float4( collider.localRotation.x, collider.localRotation.y, collider.localRotation.z, collider.localRotation.w ));
                
                switch(collider.type) {
                    case ColliderType::BOX:
                        dcColliders.BoxArgs(Windows::Foundation::Numerics::float3(collider.boxArgs.x, collider.boxArgs.y, collider.boxArgs.z));
                        break;
                    case ColliderType::CAPSULE:
                        dcColliders.CapsuleArgs(Windows::Foundation::Numerics::float2(collider.capsuleArgs.x, collider.capsuleArgs.y));
                        break;
                    case ColliderType::SPHERE:
                        dcColliders.SphereArg(collider.sphereArgs);
                        break;
                    case ColliderType::MESH:
                        dcColliders.Type(static_cast<uint32_t>(ColliderType::MESH));
                        break;
                }

                colliders.Append(dcColliders);
            }
        }

        __super::OnNavigatedTo(e);
    }
}
