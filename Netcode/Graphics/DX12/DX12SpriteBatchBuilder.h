#pragma once

#include <Netcode/HandleTypes.h>
#include <memory>

namespace Netcode::Module {
	class IGraphicsModule;
}

namespace Netcode::Graphics::DX12 {

	class SpriteBatchBuilderImpl : public SpriteBatchBuilder {
		Ptr<Module::IGraphicsModule> gModule;
		Ref<RootSignature> rootSig;
		Ref<PipelineState> pso;
	public:
		SpriteBatchBuilderImpl(Ptr<Module::IGraphicsModule> gModule);

		virtual void SetPipelineState(Ref<PipelineState> pipelineState) override;
		virtual void SetRootSignature(Ref<RootSignature> rootSignature) override;

		virtual Ref<SpriteBatch> Build() override;
	};

}
