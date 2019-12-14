#pragma once

#include "DX12Common.h"
#include <algorithm>
#include "HandleTypes.h"
#include "Utility.h"

namespace Egg::Graphics::DX12 {
	
	enum class RenderItemState : unsigned {
		UNKNOWN = 0, IN_USE = 1, RETURNED = 2
	};

	struct LODLevel {
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		UINT vertexCount;
		UINT indexCount;
	};

	__declspec(align(32)) class RenderItem {

		struct Cbuffer {
			UINT rootSigSlot;
			D3D12_GPU_VIRTUAL_ADDRESS addr;
		};

	public:
		RenderItemState state;
		HMATERIAL material;

		ID3D12RootSignature * rootSignature;
		ID3D12PipelineState * graphicsPso;

		Cbuffer cbuffers[8];
		UINT numCbuffers;

		UINT texturesRootSigSlot;
		UINT texturesId;
		UINT texturesLength;
		D3D12_GPU_DESCRIPTOR_HANDLE texturesHandle;
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology;

		UINT lodLevelsLength;
		UINT selectedLod;
		LODLevel lodLevels[4];


		inline void SetConstantBuffer(ID3D12GraphicsCommandList* gcl, UINT rootSigIdx, D3D12_GPU_VIRTUAL_ADDRESS addr) {
			if(addr == 0) {
				return;
			}
			//Egg::Utility::Debugf("Setting root parameter %d on addr: %llx\r\n", rootSigIdx, addr);
			gcl->SetGraphicsRootConstantBufferView(rootSigIdx, addr);
		}

		inline void SetPipelineState(ID3D12GraphicsCommandList * gcl) {
			gcl->SetGraphicsRootSignature(rootSignature);
			gcl->SetPipelineState(graphicsPso);

		}

		inline void SelectLOD(UINT lodLevel) {
			selectedLod = std::min(lodLevelsLength - 1, lodLevel);
		}

		inline void SetRenderItemResources(ID3D12GraphicsCommandList * gcl) {
			for(UINT i = 0; i < numCbuffers; ++i) {
				SetConstantBuffer(gcl, cbuffers[i].rootSigSlot, cbuffers[i].addr);
			}

			if(texturesHandle.ptr != 0) {
				gcl->SetGraphicsRootDescriptorTable(texturesRootSigSlot, texturesHandle);
			}
		}

		inline void Render(ID3D12GraphicsCommandList * gcl) {
			//Egg::Utility::Debugf("\r\n");
			SetPipelineState(gcl);
			SetRenderItemResources(gcl);
			DrawGeometry(gcl);
		}

		inline void DrawGeometry(ID3D12GraphicsCommandList * gcl) {
			gcl->IASetPrimitiveTopology(primitiveTopology);
			LODLevel * lvl = lodLevels + selectedLod;
			if(lvl->indexCount > 0) {
				gcl->IASetIndexBuffer(&lvl->indexBufferView);
				gcl->IASetVertexBuffers(0, 1, &lvl->vertexBufferView);
				gcl->DrawIndexedInstanced(lvl->indexCount, 1, 0, 0, 0);
			} else {
				gcl->IASetVertexBuffers(0, 1, &lvl->vertexBufferView);
				gcl->DrawInstanced(lvl->vertexCount, 1, 0, 0);
			}
		}
	};

}
