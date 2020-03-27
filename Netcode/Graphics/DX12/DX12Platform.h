#pragma once

#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class Platform {
		~Platform() = delete;
		Platform() = delete;
	public:

		static D3D_SHADER_MODEL MaxSupportedShaderModel;
		static D3D_ROOT_SIGNATURE_VERSION MaxSupportedRootSignatureVersion;

		static uint32_t RenderTargetViewIncrementSize;
		static uint32_t ShaderResourceViewIncrementSize;
		static uint32_t DepthStencilViewIncrementSize;
		static uint32_t SamplerIncrementSize;

	};

}
