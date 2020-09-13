#include "DX12ResourceDesc.h"
#include <Netcode/Graphics/ResourceEnums.h>
#include <Netcode/Graphics/ResourceDesc.h>
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {
	D3D12_PRIMITIVE_TOPOLOGY GetNativePrimitiveTopology(PrimitiveTopology t) {
		return static_cast<D3D12_PRIMITIVE_TOPOLOGY>(t);
	}
	D3D12_RESOURCE_FLAGS GetNativeFlags(ResourceFlags f) {
		return static_cast<D3D12_RESOURCE_FLAGS>(f);
	}

	D3D12_RESOURCE_DIMENSION GetNativeDimension(ResourceDimension d) {
		return static_cast<D3D12_RESOURCE_DIMENSION>(d);
	}

	D3D12_RESOURCE_STATES GetNativeState(ResourceStates s) {
		return static_cast<D3D12_RESOURCE_STATES>(static_cast<ResourceState>(s));
	}

	D3D12_HEAP_TYPE GetNativeHeapType(ResourceType r) {
		switch(r) {
			case ResourceType::PERMANENT_DEFAULT:
			case ResourceType::TRANSIENT_DEFAULT:
				return D3D12_HEAP_TYPE_DEFAULT;
			case ResourceType::PERMANENT_READBACK:
			case ResourceType::TRANSIENT_READBACK:
				return D3D12_HEAP_TYPE_READBACK;
			case ResourceType::PERMANENT_UPLOAD:
			case ResourceType::TRANSIENT_UPLOAD:
				return D3D12_HEAP_TYPE_UPLOAD;
			default:
				return D3D12_HEAP_TYPE_CUSTOM;
		}
	}

	D3D12_RESOURCE_DESC GetNativeDesc(const ResourceDesc & r) {
		D3D12_RESOURCE_DESC desc;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.DepthOrArraySize = r.depth;
		desc.Width = r.width;
		desc.Height = r.height;
		desc.Flags = GetNativeFlags(r.flags);
		desc.Dimension = GetNativeDimension(r.dimension);
		desc.MipLevels = r.mipLevels;
		desc.Format = (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? DXGI_FORMAT_UNKNOWN : r.format;
		desc.Layout = (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN;
		return desc;
	}

}

