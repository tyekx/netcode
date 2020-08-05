#pragma once

#include <Netcode/Graphics/FrameGraph.h>

#include <list>
#include <map>

namespace Netcode::Graphics::DX12 {

	class RenderPassImpl;

	class FrameGraphImpl : public FrameGraph {
	private:
		struct ResourceRegistry {
			uint32_t numReads;
			uint32_t numWrites;

			ResourceRegistry() : numReads{ 0 }, numWrites{ 0 } { }
		};

		std::map<uint64_t, ResourceRegistry> resources;
		std::list<Ref<RenderPass>> renderPasses;

	public:
		FrameGraphImpl(Vector<Ref<RenderPassImpl>> setupRenderPasses);

		FrameGraphImpl(const FrameGraphImpl &) = delete;
		FrameGraphImpl & operator=(const FrameGraphImpl &) = delete;

		virtual Vector<Ref<RenderPass>> QueryDanglingRenderPasses() override;

		virtual void EraseRenderPasses(Vector<Ref<RenderPass>> rps) override;

		virtual Vector<Ref<RenderPass>> QueryCompleteRenderPasses() override;

		virtual bool UsingBackbuffer() const override;
	};

}
