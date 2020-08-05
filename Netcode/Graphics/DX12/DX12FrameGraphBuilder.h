#pragma once

#include <Netcode/HandleDecl.h>
#include <Netcode/Graphics/FrameGraph.h>
#include <vector>
#include <memory>

namespace Netcode::Graphics {

	class IResourceContext;

}

namespace Netcode::Graphics::DX12 {

	class RenderPassImpl;

	class FrameGraphBuilderImpl : public Netcode::FrameGraphBuilder {
		std::vector<Ref<RenderPassImpl>> renderPasses;
		Ref<IResourceContext> resourceContext;

	public:
		FrameGraphBuilderImpl(Ref<IResourceContext> resourceContext);

		virtual void CreateRenderPass(const std::string & name,
			SetupCallback setupFunction,
			RenderCallback renderFunction) override;

		virtual Ref<FrameGraph> Build() override;
	};

}
