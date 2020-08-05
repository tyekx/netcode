#include "DX12GPipelineState.h"
#include "DX12Helpers.h"
#include "DX12StreamOutput.h"
#include "DX12InputLayout.h"
#include "DX12RootSignature.h"

namespace Netcode::Graphics::DX12 {

	void GPipelineStateDesc::FillNativeDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC & psd) {
		psd.CachedPSO.CachedBlobSizeInBytes = 0;
		psd.CachedPSO.pCachedBlob = nullptr;
		psd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		psd.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		psd.NodeMask = 0;
		psd.SampleMask = UINT_MAX;
		psd.SampleDesc.Count = 1;
		psd.SampleDesc.Quality = 0;

		psd.BlendState = GetNativeBlendDesc(blendState);
		psd.DepthStencilState = GetNativeDepthStencilDesc(depthStencilState);
		psd.RasterizerState = GetNativeRasterizerDesc(rasterizerState);

		psd.NumRenderTargets = numRenderTargets;
		psd.DSVFormat = dsvFormat;

		for(uint32_t i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
			psd.RTVFormats[i] = rtvFormats[i];
		}

		psd.PrimitiveTopologyType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(topologyType);
		psd.InputLayout = std::dynamic_pointer_cast<InputLayoutImpl>(inputLayout)->GetNativeInputLayout();
		psd.pRootSignature = std::dynamic_pointer_cast<RootSignatureImpl>(rootSignature)->GetNativeRootSignature();

		if(streamOutput != nullptr) {
			psd.StreamOutput = std::dynamic_pointer_cast<StreamOutputImpl>(streamOutput)->GetNativeStreamOutput();
		} else {
			psd.StreamOutput.NumEntries = 0;
			psd.StreamOutput.pBufferStrides = nullptr;
			psd.StreamOutput.pSODeclaration = nullptr;
			psd.StreamOutput.NumStrides = 0;
			psd.StreamOutput.RasterizedStream = 0;
		}

		psd.VS = GetNativeBytecode(VS.get());
		psd.PS = GetNativeBytecode(PS.get());
		psd.GS = GetNativeBytecode(GS.get());
		psd.HS = GetNativeBytecode(HS.get());
		psd.DS = GetNativeBytecode(DS.get());
	}

	/*
		Design decision: every Ref is checked for uniqueness to the best of our abilities, so now a simple pointer check should be
		adequate for them
		*/
	bool GPipelineStateDesc::operator==(const GPipelineStateDesc & rhs) const {
		if(memcmp(&blendState, &rhs.blendState, sizeof(BlendDesc)) != 0) {
			return false;
		}

		if(memcmp(&rasterizerState, &rhs.rasterizerState, sizeof(RasterizerDesc)) != 0) {
			return false;
		}

		if(memcmp(&depthStencilState, &rhs.depthStencilState, sizeof(DepthStencilDesc)) != 0) {
			return false;
		}

		if(numRenderTargets != rhs.numRenderTargets) {
			return false;
		}

		for(uint32_t i = 0; i < numRenderTargets && i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
			if(rtvFormats[i] != rhs.rtvFormats[i]) {
				return false;
			}
		}

		return rootSignature == rhs.rootSignature &&
			streamOutput == rhs.streamOutput &&
			inputLayout == rhs.inputLayout &&
			VS == rhs.VS &&
			PS == rhs.PS &&
			GS == rhs.GS &&
			HS == rhs.HS &&
			DS == rhs.DS &&
			dsvFormat == rhs.dsvFormat &&
			topologyType == rhs.topologyType;
	}

	GPipelineState::GPipelineState(ID3D12Device * device, GPipelineStateDesc && gpsoDesc) : pso{ nullptr }, psoDesc{ std::move(gpsoDesc) } {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
		psoDesc.FillNativeDesc(desc);

		DX_API("Failed to create gpso")
			device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso.GetAddressOf()));
	}

	ID3D12PipelineState * GPipelineState::GetNativePipelineState() const {
		return pso.Get();
	}

	const GPipelineStateDesc & GPipelineState::GetDesc() const {
		return psoDesc;
	}

	void * GPipelineState::GetImplDetail() const {
		return GetNativePipelineState();
	}

}
