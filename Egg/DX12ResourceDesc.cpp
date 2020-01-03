#include "DX12ResourceDesc.h"

namespace Egg::Graphics::DX12 {

	inline D3D12_RESOURCE_FLAGS GetNativeFlags(const ResourceFlags & f) {
		return static_cast<D3D12_RESOURCE_FLAGS>(f);
	}

	inline D3D12_RESOURCE_DIMENSION GetNativeDimension(const ResourceDimension & d) {
		return static_cast<D3D12_RESOURCE_DIMENSION>(d);
	}

	inline D3D12_RESOURCE_STATES GetNativeState(const ResourceState & s) {
		return static_cast<D3D12_RESOURCE_STATES>(s);
	}

	inline D3D12_HEAP_TYPE GetNativeHeapType(const ResourceDesc & r) {
		switch(r.type) {
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

	inline D3D12_RESOURCE_DESC GetNativeDesc(const ResourceDesc & r) {
		D3D12_RESOURCE_DESC desc;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.DepthOrArraySize = r.depth;
		desc.Width = r.width;
		desc.Height = r.height;
		desc.Flags = GetNativeFlags(r.flags);
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Format = r.format;
		desc.MipLevels = r.mipLevels;
		desc.Dimension = GetNativeDimension(r.dimension);
		return desc;
	}


}

