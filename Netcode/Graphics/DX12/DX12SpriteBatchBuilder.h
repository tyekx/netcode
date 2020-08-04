#pragma once

#include <Netcode/HandleTypes.h>
#include <memory>

namespace Netcode::Module {
	class IGraphicsModule;
}

namespace Netcode::Graphics::DX12 {

	class SpriteBatchBuilder : public Netcode::SpriteBatchBuilder {
		Ptr<Module::IGraphicsModule> gModule;
		Ref<Netcode::RootSignature> rootSig;
		Ref<Netcode::PipelineState> pso;
	public:
		SpriteBatchBuilder(Ptr<Module::IGraphicsModule> gModule);

		virtual void SetPipelineState(Ref<Netcode::RootSignature> pipelineState) override;
		virtual void SetRootSignature(Ref<Netcode::PipelineState> rootSignature) override;

		virtual Ref<Netcode::SpriteBatch> Build() override;
	};

}
