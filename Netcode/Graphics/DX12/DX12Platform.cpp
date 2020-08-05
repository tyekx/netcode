#include "DX12Platform.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	D3D_SHADER_MODEL Platform::MaxSupportedShaderModel;
	D3D_ROOT_SIGNATURE_VERSION Platform::MaxSupportedRootSignatureVersion;

	uint32_t Platform::RenderTargetViewIncrementSize;
	uint32_t Platform::ShaderResourceViewIncrementSize;
	uint32_t Platform::DepthStencilViewIncrementSize;
	uint32_t Platform::SamplerIncrementSize;

	static_assert(D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT == 8, "The project manually defines this value to 8, but the D3D12 value mismatches.");

}
