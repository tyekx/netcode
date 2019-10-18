#include "Material.h"

namespace Egg {

	Material::Material(Egg::PsoManager * psoMan, const D3D12_INPUT_LAYOUT_DESC & layout, Egg::PipelineState::P pipelineDesc) : gpso{ nullptr }, psoDesc{ pipelineDesc } {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc;
		ZeroMemory(&gpsoDesc, sizeof(gpsoDesc));
		gpsoDesc.NumRenderTargets = 1;
		gpsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gpsoDesc.InputLayout = layout;
		pipelineDesc->ApplyToDescriptor(gpsoDesc);

		gpso = psoMan->Get(gpsoDesc);

		memset(constantBufferAssoc, -1, sizeof(constantBufferAssoc));
	}

	Material::Material(Egg::PsoManager * psoMan, Egg::Mesh::Geometry::P geom, Egg::PipelineState::P pipelineDesc) :
		Material(psoMan, geom->GetInputLayout(), std::move(pipelineDesc)) {
	}

	void Material::ConstantBufferSlot(int cbvId, int rootSignatureSlot) {
		ASSERT(MAX_CONSTANT_BUFFER_COUNT > cbvId && cbvId >= 0, "Constant buffer index must be between 0 and MAX_CONSTANT_BUFFER_COUNT");

		ASSERT(constantBufferAssoc[cbvId] == -1, "Constant buffer slot already occupied");

		constantBufferAssoc[cbvId] = rootSignatureSlot;
	}

	void Material::ApplyPipelineState(ID3D12GraphicsCommandList * gcl) {
		gcl->SetGraphicsRootSignature(psoDesc->GetRootSignature().Get());
		gcl->SetPipelineState(gpso.Get());
	}

}
