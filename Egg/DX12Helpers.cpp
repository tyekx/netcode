#include "DX12Helpers.h"

namespace Egg::Graphics::DX12 {

	D3D12_RENDER_TARGET_BLEND_DESC GetNativeRTBlendDesc(const RenderTargetBlendDesc & rtBlend) {
		D3D12_RENDER_TARGET_BLEND_DESC rtb;
		rtb.BlendEnable = rtBlend.blendEnable;
		rtb.BlendOp = static_cast<D3D12_BLEND_OP>(rtBlend.blendOp);
		rtb.BlendOpAlpha = static_cast<D3D12_BLEND_OP>(rtBlend.blendOpAlpha);
		rtb.DestBlend = static_cast<D3D12_BLEND>(rtBlend.destBlend);
		rtb.DestBlendAlpha = static_cast<D3D12_BLEND>(rtBlend.destBlendAlpha);
		rtb.LogicOp = static_cast<D3D12_LOGIC_OP>(rtBlend.logicOp);
		rtb.LogicOpEnable = rtBlend.logicOpEnable;
		rtb.RenderTargetWriteMask = rtBlend.renderTargetWriteMask;
		rtb.SrcBlend = static_cast<D3D12_BLEND>(rtBlend.srcBlend);
		rtb.SrcBlendAlpha = static_cast<D3D12_BLEND>(rtBlend.srcBlendAlpha);
		return rtb;
	}

	D3D12_BLEND_DESC GetNativeBlendDesc(const BlendDesc & blendDesc) {
		D3D12_BLEND_DESC nativeDesc{ };

		nativeDesc.AlphaToCoverageEnable = blendDesc.alphaToCoverageEnabled;
		nativeDesc.IndependentBlendEnable = blendDesc.independentAlphaEnabled;
		for(uint8_t i = 0; i < 8; ++i) {
			nativeDesc.RenderTarget[i] = GetNativeRTBlendDesc(blendDesc.rtBlend[i]);
		}

		return nativeDesc;
	}

	D3D12_SHADER_BYTECODE GetNativeBytecode(ShaderBytecodeRef bytecode) {
		D3D12_SHADER_BYTECODE bc;
		if(bytecode != nullptr) {
			bc.pShaderBytecode = bytecode->GetBufferPointer();
			bc.BytecodeLength = bytecode->GetBufferSize();
		} else {
			bc.pShaderBytecode = nullptr;
			bc.BytecodeLength = 0;
		}
		return bc;
	}

	D3D12_DEPTH_STENCIL_DESC GetNativeDepthStencilDesc(const DepthStencilDesc & dsd) {
		D3D12_DEPTH_STENCIL_DESC nd;
		nd.BackFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(dsd.backFace.stencilDepthFailOp);
		nd.BackFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(dsd.backFace.stencilFailOp);
		nd.BackFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(dsd.backFace.stencilPassOp);
		nd.BackFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(dsd.backFace.stencilFunc);

		nd.FrontFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(dsd.frontFace.stencilDepthFailOp);
		nd.FrontFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(dsd.frontFace.stencilFailOp);
		nd.FrontFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(dsd.frontFace.stencilPassOp);
		nd.FrontFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(dsd.frontFace.stencilFunc);

		nd.StencilEnable = dsd.stencilEnable;
		nd.StencilReadMask = dsd.stencilReadMask;
		nd.StencilWriteMask = dsd.stencilWriteMask;
		nd.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(dsd.depthFunc);
		nd.DepthEnable = dsd.depthEnable;
		nd.DepthWriteMask = (dsd.depthWriteMaskZero) ? D3D12_DEPTH_WRITE_MASK_ZERO : D3D12_DEPTH_WRITE_MASK_ALL;

		return nd;
	}

	D3D12_RASTERIZER_DESC GetNativeRasterizerDesc(const RasterizerDesc & rsd) {
		D3D12_RASTERIZER_DESC nd = { };
		nd.AntialiasedLineEnable = rsd.antialiasedLineEnable;
		nd.ConservativeRaster = (rsd.conservativeRaster) ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		nd.CullMode = static_cast<D3D12_CULL_MODE>(rsd.cullMode);
		nd.FillMode = static_cast<D3D12_FILL_MODE>(rsd.fillMode);
		nd.DepthBias = rsd.depthBias;
		nd.DepthBiasClamp = rsd.depthBiasClamp;
		nd.DepthClipEnable = rsd.depthClipEnable;
		nd.ForcedSampleCount = rsd.forcedSampleCount;
		nd.FrontCounterClockwise = rsd.frontCounterClockwise;
		nd.MultisampleEnable = rsd.multisampleEnable;
		nd.SlopeScaledDepthBias = rsd.slopeScaledDepthBias;
		return nd;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(const ResourceDesc & resource)
	{
		ASSERT(resource.dimension == ResourceDimension::TEXTURE2D, "only texture2d-s are supported");

		D3D12_SHADER_RESOURCE_VIEW_DESC srvd;
		srvd.Format = resource.format;

		if(resource.depth == 6) {
			srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvd.TextureCube.MipLevels = resource.mipLevels;
			srvd.TextureCube.MostDetailedMip = 0;
			srvd.TextureCube.ResourceMinLODClamp = 0.0f;
		} else {
			srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvd.Texture2D.MipLevels = resource.mipLevels;
			srvd.Texture2D.MostDetailedMip = 0;
			srvd.Texture2D.PlaneSlice = 0;
			srvd.Texture2D.ResourceMinLODClamp = 0.0f;
		}

		switch(srvd.Format) {
			case DXGI_FORMAT_D32_FLOAT:
				srvd.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case DXGI_FORMAT_D16_UNORM:
				srvd.Format = DXGI_FORMAT_R16_UNORM;
				break;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
				srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				break;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
				srvd.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				break;
		}

		srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		return srvd;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUnorderedAccessViewDesc(const ResourceDesc & resource)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavd;
		uavd.Format = resource.format;
		uavd.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavd.Buffer.CounterOffsetInBytes = 0;
		uavd.Buffer.StructureByteStride = (uavd.Format == DXGI_FORMAT_UNKNOWN) ? resource.strideInBytes : 0;
		uavd.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavd.Buffer.FirstElement = 0;

		if(uavd.Buffer.StructureByteStride > 0) {
			uavd.Buffer.NumElements = static_cast<UINT>(resource.sizeInBytes) / uavd.Buffer.StructureByteStride;
		} else {
			uavd.Buffer.NumElements = (static_cast<UINT>(resource.sizeInBytes) / static_cast<UINT>(DirectX::BitsPerPixel(uavd.Format) / 8));
		}

		return uavd;
	}

	D3D12_RENDER_TARGET_VIEW_DESC GetRenderTargetViewDesc(const ResourceDesc & resource)
	{
		ASSERT(resource.dimension == ResourceDimension::TEXTURE2D, "Only texture2D-s are supported for now");

		D3D12_RENDER_TARGET_VIEW_DESC rtvd;
		rtvd.Format = resource.format;
		rtvd.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvd.Texture2D.MipSlice = 0;
		rtvd.Texture2D.PlaneSlice = 0;

		return rtvd;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC GetDepthStencilViewDesc(const ResourceDesc & resource)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvd;
		dsvd.Flags = D3D12_DSV_FLAG_NONE;
		dsvd.Texture2D.MipSlice = 0;
		dsvd.Format = resource.format;
		dsvd.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		ASSERT(resource.dimension == ResourceDimension::TEXTURE2D, "Only texture2D-s are supported for now");

		return dsvd;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC GetConstantBufferViewDesc(const GResource & resource)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvd;
		cbvd.BufferLocation = resource.address;
		cbvd.SizeInBytes = resource.desc.sizeInBytes;

		return cbvd;
	}
}
