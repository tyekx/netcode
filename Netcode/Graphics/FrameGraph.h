#pragma once

#include <Netcode/HandleDecl.h>
#include <string>

namespace Netcode::Graphics {
	class IResourceContext;
	class IRenderContext;
}

namespace Netcode {

	using SetupCallback = std::function<void(Graphics::IResourceContext *)>;
	using RenderCallback = std::function<void(Graphics::IRenderContext *)>;

	enum class RenderPassType {
		COMPUTE, GRAPHICS
	};

	class RenderPass {
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassType Type() const = 0;
		virtual void Type(RenderPassType value) = 0;

		virtual ArrayView<uintptr_t> GetReadResources() = 0;
		virtual ArrayView<uintptr_t> GetWrittenResources() = 0;

		virtual void ReadsResource(uintptr_t resource) = 0;
		virtual void WritesResource(uintptr_t resource) = 0;
	};


	/*
	proposed algorithm:
	Setup(...): calls the setup function of each render pass

	let v = QueryDanglingRenderPasses()
	while v is not empty do:
		EraseRenderPasses(v)
		v = QueryDanglingRenderPasses()

	v = QueryCompleteRenderPasses()

	while v is not empty do:
		foreach pass in v do:
			pass.Render(...): invokes the render function of a render pass
		EraseRenderPasses(v)
		v = QueryCompleteRenderPasses()

	by the end of this, there should not be any render pass left, all is culled or executed
	*/
	class FrameGraph {
	public:
		virtual ~FrameGraph() = default;
		/*
		Returns the set of render passes, that has no consumers but produces a resource.
		This is used for culling the framegraph. An empty vector means that there is nothing left to clear
		*/
		virtual Vector<Ref<RenderPass>> QueryDanglingRenderPasses() = 0;

		/*
		Culls the render passes, removes its connected references
		*/
		virtual void EraseRenderPasses(Vector<Ref<RenderPass>> rps) = 0;

		/*
		Returns the next render pass
		*/
		virtual Ref<RenderPass> QueryFrontRenderPass() = 0;

		/*
		Pops the front render pass
		*/
		virtual void PopRenderPass() = 0;

		/*
		Returns true if the currently remaining render passes write the null resource (backbuffer)
		*/
		virtual bool UsingBackbuffer() const = 0;
	};

	class FrameGraphBuilder {
	public:
		virtual ~FrameGraphBuilder() = default;

		virtual void CreateRenderPass(const std::string & name,
			SetupCallback setupFunction,
			RenderCallback renderFunction) = 0;

		virtual Ref<FrameGraph> Build() = 0;
	};

}

