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
	std::string name;
	SetupCallback setup;
	RenderCallback render;

	RenderPass(const std::string & name, SetupCallback && setupFunc, RenderCallback && renderFunc) : name{ name }, setup{ std::move(setupFunc) }, render{ std::move(renderFunc) } {

	}
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

class FrameGraphBuilder {
	std::vector<RenderPass> renderPasses;
public:
	RenderPass * CreateRenderPass(const std::string & name,
								   std::function<void(IResourceContext*)> setupFunction,
								   std::function<void(IRenderContext*)> renderFunction) {
		return &renderPasses.emplace_back(name, std::move(setupFunction), std::move(renderFunction));
	}

	FrameGraph Build(IResourceContext* ctx) {
		for(RenderPass & pass : renderPasses) {
			pass.setup(ctx);
		}

		return FrameGraph{ std::move(renderPasses) };
	}
};
