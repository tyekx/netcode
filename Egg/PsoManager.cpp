#include "PsoManager.h"

bool Egg::PsoManager::InputLayoutIsCompatible(const D3D12_INPUT_LAYOUT_DESC & lhs, const D3D12_INPUT_LAYOUT_DESC & rhs) {
	unsigned int lCount = lhs.NumElements;
	unsigned int rCount = rhs.NumElements;

	for(unsigned int j = 0; j < rCount; ++j) {
		bool found = false;

		for(unsigned int i = 0; i < lCount; ++i) {
			const D3D12_INPUT_ELEMENT_DESC & lDesc = lhs.pInputElementDescs[i];
			const D3D12_INPUT_ELEMENT_DESC & rDesc = rhs.pInputElementDescs[j];
			if(lDesc.SemanticIndex == rDesc.SemanticIndex && (strcmp(lDesc.SemanticName, rDesc.SemanticName) == 0)) {
				found = true;

				if(lDesc.AlignedByteOffset != rDesc.AlignedByteOffset ||
				   lDesc.Format != rDesc.Format ||
				   lDesc.InputSlot != rDesc.Format ||
				   lDesc.InputSlotClass != rDesc.InputSlotClass ||
				   lDesc.InstanceDataStepRate != rDesc.InstanceDataStepRate) {
					return false;
				}
			}
		}

		if(!found) {
			return false;
		}
	}
	return true;
}

bool Egg::PsoManager::RenderTargetsAreCompatible(const DXGI_FORMAT * lhsFormats, const DXGI_FORMAT * rhsFormats, unsigned int numRenderTargets) {
	for(unsigned int i = 0; i < numRenderTargets; ++i) {
		if(lhsFormats[i] != rhsFormats[i]) {
			return false;
		}
	}
	return true;
}

bool Egg::PsoManager::DepthStencilOpEqual(const D3D12_DEPTH_STENCILOP_DESC & lhs, const D3D12_DEPTH_STENCILOP_DESC & rhs) {
	return lhs.StencilDepthFailOp == rhs.StencilDepthFailOp &&
		lhs.StencilFailOp == rhs.StencilFailOp &&
		lhs.StencilFunc == rhs.StencilFunc &&
		lhs.StencilPassOp == rhs.StencilPassOp;
}

bool Egg::PsoManager::DepthStencilEqual(const D3D12_DEPTH_STENCIL_DESC & lhs, const D3D12_DEPTH_STENCIL_DESC & rhs) {
	if(lhs.DepthEnable == FALSE && rhs.DepthEnable == FALSE &&
	   lhs.StencilEnable == FALSE && rhs.StencilEnable == FALSE) {
		return true;
	}

	return lhs.DepthEnable == rhs.DepthEnable &&
		lhs.StencilEnable == rhs.StencilEnable &&
		DepthStencilOpEqual(lhs.FrontFace, rhs.FrontFace) &&
		DepthStencilOpEqual(lhs.BackFace, rhs.BackFace) &&
		lhs.DepthFunc == rhs.DepthFunc &&
		lhs.DepthWriteMask == rhs.DepthWriteMask &&
		lhs.StencilReadMask == rhs.StencilReadMask &&
		lhs.StencilWriteMask == rhs.StencilWriteMask;
}

bool Egg::PsoManager::RasterizerStateEqual(const D3D12_RASTERIZER_DESC & lhs, const D3D12_RASTERIZER_DESC & rhs) {
	return lhs.AntialiasedLineEnable == rhs.AntialiasedLineEnable &&
		lhs.ConservativeRaster == rhs.ConservativeRaster &&
		lhs.CullMode == rhs.CullMode &&
		lhs.DepthBias == rhs.DepthBias &&
		lhs.DepthBiasClamp == rhs.DepthBiasClamp &&
		lhs.DepthClipEnable == rhs.DepthClipEnable &&
		lhs.FillMode == rhs.FillMode &&
		lhs.ForcedSampleCount == rhs.ForcedSampleCount &&
		lhs.FrontCounterClockwise == rhs.FrontCounterClockwise &&
		lhs.MultisampleEnable == rhs.MultisampleEnable &&
		lhs.SlopeScaledDepthBias == rhs.SlopeScaledDepthBias;
}

bool Egg::PsoManager::ByteCodesEqual(const D3D12_SHADER_BYTECODE & lhs, const D3D12_SHADER_BYTECODE & rhs) {
	return lhs.BytecodeLength == rhs.BytecodeLength && lhs.pShaderBytecode == rhs.pShaderBytecode;
}

bool Egg::PsoManager::Equal(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & lhs, const D3D12_GRAPHICS_PIPELINE_STATE_DESC & rhs) {
	if(lhs.pRootSignature != rhs.pRootSignature) {
		return false;
	}

	if(!InputLayoutIsCompatible(lhs.InputLayout, rhs.InputLayout)) {
		return false;
	}

	if(!ByteCodesEqual(lhs.VS, rhs.VS) ||
	   !ByteCodesEqual(lhs.GS, rhs.GS) ||
	   !ByteCodesEqual(lhs.HS, rhs.HS) ||
	   !ByteCodesEqual(lhs.DS, rhs.DS) ||
	   !ByteCodesEqual(lhs.PS, rhs.PS)) {
		return false;
	}

	if(!DepthStencilEqual(lhs.DepthStencilState, rhs.DepthStencilState)) {
		return false;
	}

	if(!RasterizerStateEqual(lhs.RasterizerState, rhs.RasterizerState)) {
		return false;
	}


	if(lhs.DSVFormat != rhs.DSVFormat ||
	   lhs.PrimitiveTopologyType != rhs.PrimitiveTopologyType ||
	   lhs.IBStripCutValue != rhs.IBStripCutValue ||
	   lhs.Flags != rhs.Flags ||
	   lhs.NumRenderTargets != rhs.NumRenderTargets) {
		return false;
	}

	if(!RenderTargetsAreCompatible(lhs.RTVFormats, rhs.RTVFormats, lhs.NumRenderTargets)) {
		return false;
	}

	return true;
}

int Egg::PsoManager::Exists(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & sDesc) {
	int index = -1;
	for(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & i : gpsoDescs) {
		++index;
		if(Equal(i, sDesc)) {
			return index;
		}
	}
	return -1;
}

Egg::PsoManager::PsoManager(com_ptr<ID3D12Device> dev) : device{ dev } { }

com_ptr<ID3D12PipelineState> Egg::PsoManager::Get(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {
	int indexOf = Exists(gpsoDesc);

	if(indexOf == -1) {
		com_ptr<ID3D12PipelineState> pso{ nullptr };

		DX_API("PSOManager: Failed to create GPSO")
			device->CreateGraphicsPipelineState(&gpsoDesc, IID_PPV_ARGS(pso.GetAddressOf()));

		gpsos.push_back(pso);
		gpsoDescs.push_back(gpsoDesc);

		return pso;
	} else {
		return gpsos[indexOf];
	}
}

com_ptr<ID3D12PipelineState> Egg::PsoManager::Get(ID3D12RootSignature * rootSignature, const D3D12_INPUT_LAYOUT_DESC & inputLayout, const D3D12_SHADER_BYTECODE & vertexShader, const D3D12_SHADER_BYTECODE & pixelShader) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc;
	ZeroMemory(&gpsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	gpsoDesc.pRootSignature = rootSignature;
	gpsoDesc.InputLayout = inputLayout;
	gpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsoDesc.NumRenderTargets = 1;
	gpsoDesc.DepthStencilState.StencilEnable = FALSE;
	gpsoDesc.DepthStencilState.DepthEnable = FALSE;
	gpsoDesc.SampleMask = UINT_MAX;
	gpsoDesc.SampleDesc.Count = 1;
	gpsoDesc.SampleDesc.Quality = 0;
	gpsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsoDesc.VS = vertexShader;
	gpsoDesc.PS = pixelShader;

	return Get(gpsoDesc);
}
