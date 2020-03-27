#include "DX12Platform.h"

namespace Netcode::Graphics::DX12 {

	D3D_SHADER_MODEL Platform::MaxSupportedShaderModel;
	D3D_ROOT_SIGNATURE_VERSION Platform::MaxSupportedRootSignatureVersion;

	uint32_t Platform::RenderTargetViewIncrementSize;
	uint32_t Platform::ShaderResourceViewIncrementSize;
	uint32_t Platform::DepthStencilViewIncrementSize;
	uint32_t Platform::SamplerIncrementSize;

}
