#include "pch.h"
#include "DC_Lod.h"
#include "DC_Lod.g.cpp"


namespace winrt::NetcodeAssetEditor::implementation
{
    DC_Lod::DC_Lod() : vertexCount{}, indexCount{}, vertexOffset{}, indexOffset{}, vertexBufferSize{}, indexBufferSize{} {

    }

    DC_Lod::DC_Lod(uint32_t vertexCount, uint32_t indexCount, uint32_t vertexOffset, uint32_t indexOffset, uint64_t vertexBufferSize, uint64_t indexBufferSize) :
        vertexCount{ vertexCount },
        indexCount{ indexCount },
        vertexOffset{ vertexOffset },
        indexOffset{ indexOffset },
        vertexBufferSize{ vertexBufferSize },
        indexBufferSize{ indexBufferSize }
    {

    }

    winrt::event_token DC_Lod::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_Lod::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }

    uint32_t DC_Lod::VertexCount()
    {
        return vertexCount;
    }

    void DC_Lod::VertexCount(uint32_t value)
    {
        if(vertexCount != value) {
            vertexCount = value;
            propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VertexCount"));
        }
    }

    uint32_t DC_Lod::IndexCount()
    {
        return indexCount;
    }

    void DC_Lod::IndexCount(uint32_t value)
    {
        if(indexCount != value) {
            indexCount = value;
            propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"IndexCount"));
        }
    }

    uint32_t DC_Lod::VertexOffset()
    {
        return vertexOffset;
    }

    void DC_Lod::VertexOffset(uint32_t value)
    {
        if(vertexOffset != value) {
            vertexOffset = value;
            propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VertexOffset"));
        }
    }

    uint32_t DC_Lod::IndexOffset()
    {
        return indexOffset;
    }

    void DC_Lod::IndexOffset(uint32_t value)
    {
        if(indexOffset != value) {
            indexOffset = value;
            propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"IndexOffset"));
        }
    }

    uint64_t DC_Lod::VertexBufferSize()
    {
        return vertexBufferSize;
    }

    void DC_Lod::VertexBufferSize(uint64_t value)
    {
        if(vertexBufferSize != value) {
            vertexBufferSize = value;
            propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VertexBufferSize"));
        }
    }

    uint64_t DC_Lod::IndexBufferSize()
    {
        return indexBufferSize;
    }

    void DC_Lod::IndexBufferSize(uint64_t value)
    {
        if(indexBufferSize != value) {
            indexBufferSize = value;
            propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"IndexBufferSize"));
        }
    }
}
