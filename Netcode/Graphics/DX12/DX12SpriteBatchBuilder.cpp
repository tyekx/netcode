#include "DX12SpriteBatchBuilder.h"
#include "DX12SpriteBatch.h"

namespace Netcode::Graphics::DX12 {

	SpriteBatchBuilder::SpriteBatchBuilder(Ptr<Module::IGraphicsModule> gModule) : gModule{ gModule } {

	}

	void SpriteBatchBuilder::SetPipelineState(Ref<Netcode::PipelineState> pipelineState) {
		std::swap(pso, pipelineState);
	}

	void SpriteBatchBuilder::SetRootSignature(Ref<Netcode::RootSignature> rootSignature) {
		std::swap(rootSig, rootSignature);
	}

	Ref<Netcode::SpriteBatch> SpriteBatchBuilder::Build() {
		return std::make_shared<DX12::SpriteBatch>(gModule, std::move(rootSig), std::move(pso));
	}

}
