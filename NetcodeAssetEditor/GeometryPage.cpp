#include "pch.h"
#include "GeometryPage.h"
#if __has_include("GeometryPage.g.cpp")
#include "GeometryPage.g.cpp"
#endif

#include "XamlGlobal.h"


using namespace winrt;
using namespace Windows::UI::Xaml;

static DirectX::BoundingBox CalculateBoundingBoxForModel(const std::vector<Netcode::Intermediate::Mesh> & meshes) {
    if(meshes.empty()) {
        return DirectX::BoundingBox{};
    }

    DirectX::BoundingBox boundingBox = meshes.front().boundingBox;

    for(auto it = std::cbegin(meshes) + 1; it != std::cend(meshes); ++it) {
        boundingBox = MergeBoundingBoxes(boundingBox, it->boundingBox);
    }

    return boundingBox;
}

namespace winrt::NetcodeAssetEditor::implementation
{
    void GeometryPage::UpdateRenderedModel()
    {
        auto inspectableVector = geometryListView().SelectedRanges();

        uint32_t selectedItemsCount = geometryListView().SelectedItems().Size();

        if(selectedItemsCount == 0) {
            Global::EditorApp->SetDrawGeometry({});
            Global::EditorApp->InvalidateFrame();
            return;
        }

        std::vector<Netcode::Intermediate::LOD *> drawCandidates;
        drawCandidates.reserve(selectedItemsCount);

        for(uint32_t i = 0; i < inspectableVector.Size(); ++i) {
            auto range = inspectableVector.GetAt(i);

            for(uint32_t j = range.FirstIndex(); j <= range.LastIndex(); ++j) {
                Netcode::Intermediate::LOD * lodRef = &transformedMeshes[j].lods.at(0);

                drawCandidates.push_back(lodRef);
            }
        }

        Global::EditorApp->SetDrawGeometry(std::move(drawCandidates));
        Global::EditorApp->InvalidateFrame();
    }

    void GeometryPage::OnModelChanged(uint64_t value) {
        if(Global::Model != nullptr) {
            transformedMeshes.clear();
            transformedMeshes.reserve(Global::Model->meshes.size());

            for(const Netcode::Intermediate::Mesh & m : Global::Model->meshes) {
                transformedMeshes.emplace_back(std::move(m.Clone()));
            }

            auto dc = DataContext().as<DC_GeometryPage>();
            dc->TransformBuffer().Clear();

            if(!Global::Model->meshes.empty()) {
                auto boundingBox = CalculateBoundingBoxForModel(transformedMeshes);

                dc->BoundingBoxSize(Windows::Foundation::Numerics::float3{
                    2.0f * boundingBox.Extents.x,
                    2.0f * boundingBox.Extents.y,
                    2.0f * boundingBox.Extents.z
                });

                Global::EditorApp->SetBoundingBoxes({ boundingBox });
            }
        }
    }

    GeometryPage::GeometryPage()
    {
        InitializeComponent();
        firstNavigation = true;
    }

    void GeometryPage::ListView_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        UpdateRenderedModel();
    }

    void GeometryPage::OnTransformBufferChanged(Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Numerics::float4x4> sender, Windows::Foundation::Collections::IVectorChangedEventArgs args) {
        namespace wn = Windows::Foundation::Numerics;

        wn::float4x4 transform = wn::float4x4::identity();

        for(wn::float4x4 i : sender) {
            transform = transform * i;
        }

        transform = wn::transpose(transform);

        Netcode::Float4x4 v{
            &transform.m11
        };

        Global::EditorApp->currentOfflineTransform = v;

        v = Netcode::Matrix{ Global::Model->offlineTransform } *v;

        std::vector<Netcode::Intermediate::Mesh> cloned;
        cloned.reserve(Global::Model->meshes.size());

        for(const Netcode::Intermediate::Mesh & m : Global::Model->meshes) {
            Netcode::Intermediate::Mesh cMesh = m.Clone();
            cMesh.ApplyTransformation(v);
            cloned.emplace_back(std::move(cMesh));
        }

        transformedMeshes = std::move(cloned);

        auto boundingBox = CalculateBoundingBoxForModel(transformedMeshes);
        auto dc = DataContext().as<NetcodeAssetEditor::DC_GeometryPage>();

        dc.BoundingBoxSize(Windows::Foundation::Numerics::float3{
            2.0f * boundingBox.Extents.x,
            2.0f * boundingBox.Extents.y,
            2.0f * boundingBox.Extents.z
        });

        Global::EditorApp->SetBoundingBoxes({ boundingBox });
        UpdateRenderedModel();
    }

    void GeometryPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        if(firstNavigation) {
            namespace wc = Windows::Foundation::Collections;
            namespace wn = Windows::Foundation::Numerics;

            MainPage mainPage = e.Parameter().as<MainPage>();

            mainPage.ModelChanged(NetcodeAssetEditor::ModelChangedHandler{ this, &GeometryPage::OnModelChanged });

            auto dataContext = DataContext().as<DC_GeometryPage>();
            dataContext->Shared(mainPage.DataContext().as<NetcodeAssetEditor::DC_MainPage>());
            dataContext->TransformBuffer().VectorChanged(wc::VectorChangedEventHandler<wn::float4x4>{ 
                this,
                &GeometryPage::OnTransformBufferChanged
            });

            geometryListView().SelectAll();

            firstNavigation = false;
        }
        __super::OnNavigatedTo(e);
    }

    void GeometryPage::geometryListView_ContainerContentChanging(Windows::UI::Xaml::Controls::ListViewBase const & sender, Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs const & args)
    {

    }
}

