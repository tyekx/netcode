#pragma once

#include "Common.h"

namespace Egg::Graphics::DX12 {
	
	enum class RenderItemState : unsigned {
		UNKNOWN = 0, IN_USE = 1, RETURNED = 2
	};

	__declspec(align(32)) class RenderItem {
	public:
		RenderItemState state;
		UINT ownerHandle;

		ID3D12RootSignature * rootSignature;
		ID3D12PipelineState * graphicsPso;

		int * cbAssoc;
		UINT cbAssocLength;

		D3D12_GPU_VIRTUAL_ADDRESS perObjectCbAddr;
		D3D12_GPU_VIRTUAL_ADDRESS boneDataCbAddr;

		UINT texturesRootSigSlot;
		D3D12_GPU_DESCRIPTOR_HANDLE texturesHandle;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		D3D12_GPU_VIRTUAL_ADDRESS perMeshCbAddr;
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology;

		UINT vertexCount;
		UINT indexCount;
	};

}
