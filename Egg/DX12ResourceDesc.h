#pragma once

#include "ResourceDesc.h"
#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

	class GResource {
	public:
		Egg::Graphics::ResourceDesc desc;
		ID3D12Resource * resource;
		D3D12_GPU_VIRTUAL_ADDRESS address;
	};

	D3D12_RESOURCE_FLAGS GetNativeFlags(const ResourceFlags & f);

	D3D12_RESOURCE_DIMENSION GetNativeDimension(const ResourceDimension & d);

	D3D12_RESOURCE_STATES GetNativeState(const ResourceState & s);

	D3D12_HEAP_TYPE GetNativeHeapType(const ResourceDesc & r);

	D3D12_RESOURCE_DESC GetNativeDesc(const ResourceDesc & r);

}
