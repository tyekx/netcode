#pragma once

#include "../../HandleTypes.h"
#include "../../Modules.h"

namespace Netcode::Graphics::DX12 {

	class SpriteBatchBuilder : public Netcode::SpriteBatchBuilder {
		const Netcode::Module::IGraphicsModule * gModule;
		Netcode::RootSignatureRef rootSig;
		Netcode::PipelineStateRef pso;
	public:
		SpriteBatchBuilder(const Netcode::Module::IGraphicsModule * gModule);

		virtual void SetPipelineState(PipelineStateRef pipelineState) override;

		virtual void SetRootSignature(RootSignatureRef rootSignature) override;

		virtual SpriteBatchRef Build() override;
	};

	using DX12SpriteBatchBuilder = Netcode::Graphics::DX12::SpriteBatchBuilder;

}
