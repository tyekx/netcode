#include "DX12SpriteBatchBuilder.h"
#include "DX12SpriteBatch.h"

namespace Netcode::Graphics::DX12 {

	SpriteBatchBuilderImpl::SpriteBatchBuilderImpl(Ptr<Module::IGraphicsModule> gModule) : gModule{ gModule } {

	}

	void SpriteBatchBuilderImpl::SetPipelineState(Ref<Netcode::PipelineState> pipelineState) {
		std::swap(pso, pipelineState);
	}

	void SpriteBatchBuilderImpl::SetRootSignature(Ref<RootSignature> rootSignature) {
		std::swap(rootSig, rootSignature);
	}

	Ref<SpriteBatch> SpriteBatchBuilderImpl::Build() {
		return std::make_shared<SpriteBatchImpl>(gModule, std::move(rootSig), std::move(pso));
	}

}
