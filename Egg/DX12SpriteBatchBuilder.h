#pragma once

#include "HandleTypes.h"
#include "Modules.h"

namespace Egg::Graphics::DX12 {

	class SpriteBatchBuilder : public Egg::SpriteBatchBuilder {
		const Egg::Module::IGraphicsModule * gModule;
		Egg::RootSignatureRef rootSig;
		Egg::PipelineStateRef pso;
	public:
		SpriteBatchBuilder(const Egg::Module::IGraphicsModule * gModule);

		virtual void SetPipelineState(PipelineStateRef pipelineState) override;

		virtual void SetRootSignature(RootSignatureRef rootSignature) override;

		virtual SpriteBatchRef Build() override;
	};

	using DX12SpriteBatchBuilder = Egg::Graphics::DX12::SpriteBatchBuilder;

}
