#pragma once

#include "../../HandleTypes.h"
#include "DX12ResourceDesc.h"
#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

	D3D12_RENDER_TARGET_BLEND_DESC GetNativeRTBlendDesc(const RenderTargetBlendDesc & rtBlend);

	D3D12_BLEND_DESC GetNativeBlendDesc(const BlendDesc & blendDesc);

	D3D12_SHADER_BYTECODE GetNativeBytecode(ShaderBytecodeRef bytecode);

	D3D12_DEPTH_STENCIL_DESC GetNativeDepthStencilDesc(const DepthStencilDesc & dsd);

	D3D12_RASTERIZER_DESC GetNativeRasterizerDesc(const RasterizerDesc & rsd);

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(const ResourceDesc & resource);

	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc(const ResourceDesc & resource);

	D3D12_RENDER_TARGET_VIEW_DESC GetRenderTargetViewDesc(const ResourceDesc & resource);

	D3D12_DEPTH_STENCIL_VIEW_DESC GetDepthStencilViewDesc(const ResourceDesc & resource);

	D3D12_CONSTANT_BUFFER_VIEW_DESC GetConstantBufferViewDesc(const GResource & resource);

}
