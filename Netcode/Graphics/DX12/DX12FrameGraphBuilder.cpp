#include "DX12FrameGraphBuilder.h"
#include <functional>
#include <Netcode/Graphics/GraphicsContexts.h>
#include "DX12RenderPass.h"
#include "DX12FrameGraph.h"

namespace Netcode::Graphics::DX12 {


	FrameGraphBuilderImpl::FrameGraphBuilderImpl(Ref<IResourceContext> resourceContext) : renderPasses{}, resourceContext{ std::move(resourceContext) } {

	}

	void FrameGraphBuilderImpl::CreateRenderPass(const std::string & name, SetupCallback setupFunction, RenderCallback renderFunction) {
		renderPasses.emplace_back(std::make_shared<RenderPassImpl>(name, setupFunction, renderFunction));
	}

	Ref<FrameGraph> FrameGraphBuilderImpl::Build() {
		for(auto & renderPass : renderPasses) {
			resourceContext->SetRenderPass(renderPass);
			renderPass->Setup(resourceContext.get());
		}
		resourceContext->ClearRenderPass();

		return std::make_shared<FrameGraphImpl>(std::move(renderPasses));
	}

}
