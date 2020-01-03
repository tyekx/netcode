#pragma once

#include <functional>
#include <dxgiformat.h>
#include <tuple>

#include "GraphicsContexts.h"

using Egg::Graphics::IResourceContext;
using Egg::Graphics::IRenderContext;

using SetupCallback = std::function<void(IResourceContext *)>;
using RenderCallback = std::function<void(IRenderContext *)>;

class RenderPass {
public:
	SetupCallback setup;
	RenderCallback render;
};



class FrameGraph {
	std::vector<RenderPass> renderPasses;
public:
	FrameGraph(std::vector<RenderPass> passes) : renderPasses{ std::move(passes) } {

	}

	void Render(IRenderContext * ctx) {
		for(RenderPass & pass : renderPasses) {
			ctx->BeginPass();
			pass.render(ctx);
			ctx->EndPass();
		}
	}
};

template<typename T>
class ScratchBuffer {
	std::vector<T> storage;
public:

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

	void Produced(const T & v) {
		storage.push_back(v);
	}

	void Merge(const ScratchBuffer<T> & rhs) {
		storage.insert(storage.end(), rhs.begin(), rhs.end());
	}

	std::vector<T>::iterator begin() noexcept {
		return storage.begin();
	}

	std::vector<T>::iterator end() noexcept {
		return storage.end();
	}

	std::vector<T>::const_iterator begin() const noexcept {
		return storage.begin();
	}

	std::vector<T>::const_iterator end() const noexcept {
		return storage.end();
	}

};

class FrameGraphBuilder {
	std::vector<RenderPass> renderPasses;
public:
	RenderPass * CreateRenderPass(const std::string & name,
								   std::function<void(IResourceContext*)> setupFunction,
								   std::function<void(IRenderContext*)> renderFunction) {
		return &renderPasses.emplace_back(std::move(setupFunction), std::move(renderFunction));
	}

	FrameGraph Build(IResourceContext* ctx) {
		for(RenderPass & pass : renderPasses) {
			pass.setup(ctx);
		}

		return FrameGraph{ std::move(renderPasses) };
	}
};
