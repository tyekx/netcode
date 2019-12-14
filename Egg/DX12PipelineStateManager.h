#pragma once

#include "DX12Common.h"
#include "HandleTypes.h"

namespace Egg::Graphics::DX12 {

	class PipelineStateManager {

		struct Item {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
		};

		std::vector<Item> items;


		Item & Get(HPSO handle) {
			ASSERT(handle >= 0 && handle < items.size(), "Out of range");

			return items.at(handle);
		}

		void Initialize(Item & i) {
			ZeroMemory(&i.desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			SetNumRenderTargets(i, 1);
			SetRenderTargetFormat(i, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
			SetDepthStencilFormat(i, DXGI_FORMAT_D32_FLOAT);
			SetPrimitiveTopologyType(i, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

			i.desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			i.desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			i.desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

			i.desc.SampleMask = UINT_MAX;
			i.desc.SampleDesc.Count = 1;
			i.desc.SampleDesc.Quality = 0;
			i.desc.NodeMask = 0;
		}

		void SetRenderTargetFormat(Item & i, unsigned int renderTargetIndex, DXGI_FORMAT format) {
			ASSERT(renderTargetIndex >= 0 && renderTargetIndex < 8, "Out of range, only 8 render targets are available");
			renderTargetIndex &= 0b111;
			i.desc.RTVFormats[renderTargetIndex] = format;
		}

		void SetNumRenderTargets(Item & i, unsigned int numRenderTargets) {
			i.desc.NumRenderTargets = numRenderTargets;
		}

		void SetDepthStencilFormat(Item & i, DXGI_FORMAT format) {
			ASSERT(format == DXGI_FORMAT_D16_UNORM ||
				   format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
				   format == DXGI_FORMAT_D32_FLOAT ||
				   format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT, "Depth stencil format must be a D prefixed format");

			i.desc.DSVFormat = format;
		}

		D3D12_SHADER_BYTECODE GetShaderBytecode(ID3DBlob * blob) {
			D3D12_SHADER_BYTECODE bytecode;
			bytecode.pShaderBytecode = (blob != nullptr) ? blob->GetBufferPointer() : nullptr;
			bytecode.BytecodeLength = (blob != nullptr) ? blob->GetBufferSize() : 0;
			return bytecode;
		}

		void SetPixelShader(Item & i, ID3DBlob * blob) {
			i.desc.PS = GetShaderBytecode(blob);
		}

		void SetVertexShader(Item & i, ID3DBlob * blob) {
			i.desc.VS = GetShaderBytecode(blob);
		}

		void SetGeometryShader(Item & i, ID3DBlob * blob) {
			i.desc.GS = GetShaderBytecode(blob);
		}

		void SetHullShader(Item & i, ID3DBlob * blob) {
			i.desc.HS = GetShaderBytecode(blob);
		}

		void SetDomainShader(Item & i, ID3DBlob * blob) {
			i.desc.DS = GetShaderBytecode(blob);
		}

		void SetPrimitiveTopologyType(Item & i, D3D12_PRIMITIVE_TOPOLOGY_TYPE type) {
			i.desc.PrimitiveTopologyType = type;
		}

		void SetRootSignature(Item & i, ID3D12RootSignature * rootSig) {
			i.desc.pRootSignature = rootSig;
		}

		void SetInputLayout(Item & i, const D3D12_INPUT_LAYOUT_DESC & inputLayout) {
			i.desc.InputLayout = inputLayout;
		}

	public:

		void SetRenderTargetFormat(HPSO handle, unsigned int renderTargetIndex, DXGI_FORMAT format) {
			SetRenderTargetFormat(Get(handle), renderTargetIndex, format);
		}

		void SetNumRenderTargets(HPSO handle, unsigned int numRenderTargets) {
			SetNumRenderTargets(Get(handle), numRenderTargets);
		}

		void SetDepthStencilFormat(HPSO handle, DXGI_FORMAT format) {
			SetDepthStencilFormat(Get(handle), format);
		}

		void SetPixelShader(HPSO handle, ID3DBlob* blob) {
			SetPixelShader(Get(handle), blob);
		}

		void SetVertexShader(HPSO handle, ID3DBlob * blob) {
			SetVertexShader(Get(handle), blob);
		}

		void SetGeometryShader(HPSO handle, ID3DBlob * blob) {
			SetGeometryShader(Get(handle), blob);
		}

		void SetHullShader(HPSO handle, ID3DBlob * blob) {
			SetHullShader(Get(handle), blob);
		}

		void SetDomainShader(HPSO handle, ID3DBlob * blob) {
			SetDomainShader(Get(handle), blob);
		}

		void SetPrimitiveTopologyType(HPSO handle, D3D12_PRIMITIVE_TOPOLOGY_TYPE type) {
			SetPrimitiveTopologyType(Get(handle), type);
		}

		void SetRootSignature(HPSO handle, ID3D12RootSignature * rootSig) {
			SetRootSignature(Get(handle), rootSig);
		}

		void SetInputLayout(HPSO handle, const D3D12_INPUT_LAYOUT_DESC & inputLayout) {
			SetInputLayout(Get(handle), inputLayout);
		}

		const D3D12_GRAPHICS_PIPELINE_STATE_DESC & GetGPSO(HPSO pso) {
			Item & i = Get(pso);

			return i.desc;
		}

		HPSO Create() {
			Item item;
			Initialize(item);

			HPSO idx = static_cast<HPSO>(items.size());

			items.emplace_back(item);

			return idx;
		}
	};

}
