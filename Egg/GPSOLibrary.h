#pragma once

#include "ShaderCodeCollection.h"
#include "Geometry.h"

namespace Egg::Graphics::Internal {

	class GPSOLibrary {

		ID3D12Device * device;

		struct StorageItem {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc;
			com_ptr<ID3D12PipelineState> pipelineState;
		};

		std::vector<StorageItem> items;

		bool Exist(D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {
			for(auto & i : items) {
				if(memcmp(&i.gpsoDesc, &gpsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC)) == 0) {
					return true;
				}
			}
			return false;
		}

		void CreateGPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {
			StorageItem si;
			si.gpsoDesc = gpsoDesc;
			
			DX_API("Failed to create pipeline state")
				device->CreateGraphicsPipelineState(&si.gpsoDesc, IID_PPV_ARGS(si.pipelineState.GetAddressOf()));

			items.push_back(si);
		}

		ID3D12PipelineState * GetPSOReference(D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {
			for(auto & i : items) {
				if(memcmp(&i.gpsoDesc, &gpsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC)) == 0) {
					return i.pipelineState.Get();
				}
			}
			return nullptr;
		}

	public:
		void CreateResources(ID3D12Device * dev) {
			device = dev;
		}

		ID3D12PipelineState * GetPipelineState(ID3D12RootSignature * rootSig, ShaderCodeCollection & scc, Geometry * geometry) {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gpso;
			ZeroMemory(&gpso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			gpso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			gpso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			gpso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			gpso.NumRenderTargets = 1;
			gpso.SampleMask = UINT_MAX;
			gpso.SampleDesc.Count = 1;
			gpso.SampleDesc.Quality = 0;
			gpso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			gpso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			gpso.RasterizerState.FillMode = geometry->mode;
			gpso.InputLayout = geometry->GetInputLayout();
			gpso.pRootSignature = rootSig;
			gpso.VS = CD3DX12_SHADER_BYTECODE(scc.vertexShader);
			gpso.PS = CD3DX12_SHADER_BYTECODE(scc.pixelShader);
			gpso.PrimitiveTopologyType = geometry->topologyType;

			if(!Exist(gpso)) {
				CreateGPSO(gpso);
			}

			return GetPSOReference(gpso);
		}
	};

}
