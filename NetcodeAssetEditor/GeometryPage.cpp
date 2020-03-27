#include "pch.h"
#include "GeometryPage.h"
#if __has_include("GeometryPage.g.cpp")
#include "GeometryPage.g.cpp"
#endif
#include "Model.h"
#include "XamlGlobal.h"


using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NetcodeAssetEditor::implementation
{
    GeometryPage::GeometryPage()
    {
        InitializeComponent();
        firstNavigation = true;
    }

    void GeometryPage::ListView_SelectionChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e)
    {
        auto inspectableVector = geometryListView().SelectedRanges();

        uint32_t selectedItemsCount = geometryListView().SelectedItems().Size();

        if(selectedItemsCount == 0) {
            Global::EditorApp->SetDrawGeometry({});
            Global::EditorApp->Run();
            return;
        }

        std::vector<LOD *> drawCandidates;
        drawCandidates.reserve(selectedItemsCount);

        for(uint32_t i = 0; i < inspectableVector.Size(); ++i) {
            auto range = inspectableVector.GetAt(i);

            for(uint32_t j = range.FirstIndex(); j <= range.LastIndex(); ++j) {
                LOD * lodRef = &Global::Model->meshes[j].lods.at(0);

                drawCandidates.push_back(lodRef);
            }
        }

        Global::EditorApp->SetDrawGeometry(std::move(drawCandidates));
        Global::EditorApp->Run();
    }

    void GeometryPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        if(firstNavigation) {
            DataContext(e.Parameter());

            geometryListView().SelectAll();

            firstNavigation = false;
        }
        __super::OnNavigatedTo(e);
    }

    void GeometryPage::geometryListView_ContainerContentChanging(Windows::UI::Xaml::Controls::ListViewBase const & sender, Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs const & args)
    {
        //geometryListView().SelectAll();
    }
}

