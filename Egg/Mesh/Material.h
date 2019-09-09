#pragma once

#include "../Common.h"

namespace Egg {
	namespace Mesh {

		GG_CLASS(Material)

			com_ptr<ID3D12RootSignature> rootSignature;
			com_ptr<ID3DBlob> vertexShader;
			com_ptr<ID3DBlob> geometryShader;
			com_ptr<ID3DBlob> pixelShader;
			D3D12_BLEND_DESC blendState;
			D3D12_RASTERIZER_DESC rasterizerState;
			D3D12_DEPTH_STENCIL_DESC depthStencilState;
			DXGI_FORMAT dsvFormat;
		public:

			Material() : rootSignature{ nullptr }, vertexShader{ nullptr }, geometryShader{ nullptr }, pixelShader{ nullptr }, blendState{}, rasterizerState{}, depthStencilState{} {
				blendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				depthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				depthStencilState.DepthEnable = FALSE;
				depthStencilState.StencilEnable = FALSE;
				dsvFormat = DXGI_FORMAT_UNKNOWN;
			}

			void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC & dsd) {
				depthStencilState = dsd;
			}

			void SetRasterizerState(const D3D12_RASTERIZER_DESC & rsd) {
				rasterizerState = rsd;
			}

			void SetBlendState(const D3D12_BLEND_DESC & bsd) {
				blendState = bsd;
			}

			void SetRootSignature(com_ptr<ID3D12RootSignature> rs) {
				rootSignature = rs;
			}

			void SetVertexShader(com_ptr<ID3DBlob> vs) {
				vertexShader = vs;
			}

			void SetPixelShader(com_ptr<ID3DBlob> ps) {
				pixelShader = ps;
			}

			void SetGeometryShader(com_ptr<ID3DBlob> gs) {
				geometryShader = gs;
			}

			void SetDSVFormat(DXGI_FORMAT format) {
				dsvFormat = format;
			}

			void ApplyToDescriptor(D3D12_GRAPHICS_PIPELINE_STATE_DESC & psoDesc) {
				psoDesc.pRootSignature = rootSignature.Get();
				psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
				if(geometryShader != nullptr) {
					psoDesc.GS = CD3DX12_SHADER_BYTECODE(geometryShader.Get());
				}
				psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
				psoDesc.BlendState = blendState;
				psoDesc.RasterizerState = rasterizerState;
				psoDesc.DepthStencilState = depthStencilState;
				psoDesc.DSVFormat = dsvFormat;
				psoDesc.SampleDesc.Count = 1;
				psoDesc.SampleDesc.Quality = 0;
				psoDesc.SampleMask = UINT_MAX;
			}


		GG_ENDCLASS

	}
}
