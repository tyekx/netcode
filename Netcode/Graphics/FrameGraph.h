#pragma once

#include <functional>
#include <tuple>

#include <NetcodeFoundation/ArrayView.hpp>
#include "GraphicsContexts.h"

namespace Netcode::Graphics {
	class IResourceContext;
	class IRenderContext;
}

namespace Netcode {

	using SetupCallback = std::function<void(Netcode::Graphics::IResourceContext *)>;
	using RenderCallback = std::function<void(Netcode::Graphics::IRenderContext *)>;

	class RenderPass {
	public:
		std::string name;
		SetupCallback Setup;
		RenderCallback Render;

		RenderPass(const std::string & name, SetupCallback setupFunc, RenderCallback renderFunc);
		RenderPass() = default;
		virtual ~RenderPass() = default;


		virtual bool IsComputePass() const = 0;
		virtual void IsComputePass(bool value) = 0;

		virtual Netcode::ArrayView<uint64_t> GetReadResources() = 0;
		virtual Netcode::ArrayView<uint64_t> GetWrittenResources() = 0;

		virtual void ReadsResource(uint64_t resource) = 0;
		virtual void WritesResource(uint64_t resource) = 0;
	};

	using RenderPassRef = std::shared_ptr<RenderPass>;


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
		virtual std::vector<RenderPassRef> QueryDanglingRenderPasses() = 0;

		/*
		Culls the render passes, removes its connected references
		*/
		virtual void EraseRenderPasses(std::vector<RenderPassRef> rps) = 0;

		/*
		Returns the set of render passes, that are ready to be executed
		*/
		virtual std::vector<RenderPassRef> QueryCompleteRenderPasses() = 0;
	};

	using FrameGraphRef = std::shared_ptr<FrameGraph>;

	class FrameGraphBuilder {
	public:
		virtual ~FrameGraphBuilder() = default;

		virtual void CreateRenderPass(const std::string & name,
			SetupCallback setupFunction,
			RenderCallback renderFunction) = 0;

		virtual FrameGraphRef Build() = 0;
	};

	using FrameGraphBuilderRef = std::shared_ptr<FrameGraphBuilder>;

	template<typename T>
	class ScratchBuffer {
		std::vector<T> storage;
	public:

		using iterator = typename std::vector<T>::iterator;
		using const_iterator = typename std::vector<T>::const_iterator;

		size_t Size() const {
			return storage.size();
		}

		void Expect(size_t numElements) {
			size_t currentCap = storage.capacity();
			size_t newSize = storage.size() + numElements;

			if(currentCap > newSize) {
				return;
			}

			storage.reserve(newSize);
		}

		void Clear() {
			storage.clear();
		}

		void Produced(const T & v) {
			storage.push_back(v);
		}

		void Merge(const ScratchBuffer<T> & rhs) {
			storage.insert(storage.end(), rhs.begin(), rhs.end());
		}

		iterator begin() noexcept {
			return storage.begin();
		}

		iterator end() noexcept {
			return storage.end();
		}

		const_iterator begin() const noexcept {
			return storage.begin();
		}

		const_iterator end() const noexcept {
			return storage.end();
		}
	};

}

