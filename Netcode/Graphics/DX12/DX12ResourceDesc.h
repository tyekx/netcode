#pragma once

#include "../ResourceDesc.h"

#include "DX12Common.h"
#include <tuple>

namespace Netcode::Graphics::DX12 {

	D3D12_PRIMITIVE_TOPOLOGY GetNativePrimitiveTopology(PrimitiveTopology t);

	D3D12_RESOURCE_FLAGS GetNativeFlags(ResourceFlags f);

	D3D12_RESOURCE_DIMENSION GetNativeDimension(ResourceDimension d);

	D3D12_RESOURCE_STATES GetNativeState(ResourceState s);

	D3D12_HEAP_TYPE GetNativeHeapType(ResourceType r);

	D3D12_RESOURCE_DESC GetNativeDesc(const ResourceDesc & r);

}
