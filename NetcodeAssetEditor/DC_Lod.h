#pragma once
#include "DC_Lod.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_Lod : DC_LodT<DC_Lod>
    {
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        uint32_t vertexCount;
        uint32_t indexCount;
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint64_t vertexBufferSize;
        uint64_t indexBufferSize;

        DC_Lod();
        DC_Lod(uint32_t vertexCount, uint32_t indexCount, uint32_t vertexOffset, uint32_t indexOffset, uint64_t vertexBufferSize, uint64_t indexBufferSize);
        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);

        void PropertyChanged(winrt::event_token const& token) noexcept;
        uint32_t VertexCount();
        void VertexCount(uint32_t value);
        uint32_t IndexCount();
        void IndexCount(uint32_t value);
        uint32_t VertexOffset();
        void VertexOffset(uint32_t value);
        uint32_t IndexOffset();
        void IndexOffset(uint32_t value);
        uint64_t VertexBufferSize();
        void VertexBufferSize(uint64_t value);
        uint64_t IndexBufferSize();
        void IndexBufferSize(uint64_t value);
    };
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_Lod : DC_LodT<DC_Lod, implementation::DC_Lod>
    {
    };
}
