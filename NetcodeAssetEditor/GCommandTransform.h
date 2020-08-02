#pragma once
#include "GCommandTransform.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct GCommandTransform : GCommandTransformT<GCommandTransform>
    {
        winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::IInspectable>> canExecuteChanged;

        GCommandTransform() = default;

        winrt::event_token CanExecuteChanged(Windows::Foundation::EventHandler<Windows::Foundation::IInspectable> const& handler);
        void CanExecuteChanged(winrt::event_token const& token) noexcept;
        bool CanExecute(Windows::Foundation::IInspectable const& parameter);
        void Execute(Windows::Foundation::IInspectable const& parameter);
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct GCommandTransform : GCommandTransformT<GCommandTransform, implementation::GCommandTransform>
    {
    };
}
