#include "DX12SpriteBatchBuilder.h"
#include "DX12SpriteBatch.h"

namespace Netcode::Graphics::DX12 {

	SpriteBatchBuilder::SpriteBatchBuilder(const Netcode::Module::IGraphicsModule * gModule) : gModule{ gModule } {

	}

	void SpriteBatchBuilder::SetPipelineState(PipelineStateRef pipelineState) {
		std::swap(pso, pipelineState);
	}

	void SpriteBatchBuilder::SetRootSignature(RootSignatureRef rootSignature) {
		std::swap(rootSig, rootSignature);
	}

	SpriteBatchRef SpriteBatchBuilder::Build() {
		return std::make_shared<DX12SpriteBatch>(gModule, std::move(rootSig), std::move(pso));
	}

}
