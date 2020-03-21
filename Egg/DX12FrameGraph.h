#pragma once

#include "FrameGraph.h"
#include "LinearAllocator.h"
#include <EggFoundation/ArrayView.hpp>

namespace Egg::Graphics::DX12 {

	class RenderPass : public Egg::RenderPass {
	protected:
		uint64_t read[32];
		uint64_t written[32];
		uint32_t numRead;
		uint32_t numWritten;
		bool isComputePass;

	public:
		
		RenderPass(const std::string & name, Egg::SetupCallback setup, Egg::RenderCallback render) : Egg::RenderPass(name, setup, render),
			read{}, written{}, numRead{ 0 }, numWritten{ 0 }, isComputePass{ false } {

		}

		virtual void IsComputePass(bool value) override {
			isComputePass = value;
		}

		virtual bool IsComputePass() const {
			return isComputePass;
		}

		virtual void ReadsResource(uint64_t resource) override {
			read[numRead++] = resource;
		}

		virtual void WritesResource(uint64_t resource) override {
			written[numWritten++] = resource;
		}

		virtual Egg::ArrayView<uint64_t> GetReadResources() override {
			return Egg::ArrayView<uint64_t>(read, static_cast<size_t>(numRead));
		}

		virtual Egg::ArrayView<uint64_t> GetWrittenResources() override {
			return Egg::ArrayView<uint64_t>(written, static_cast<size_t>(numWritten));
		}
	};

	using DX12RenderPass = Egg::Graphics::DX12::RenderPass;
	using DX12RenderPassRef = std::shared_ptr<DX12RenderPass>;

	class FrameGraph : public Egg::FrameGraph {
	private:
		struct ResourceRegistry {
			uint32_t numReads;
			uint32_t numWrites;

			ResourceRegistry() : numReads{ 0 }, numWrites{ 0 } { }
		};

		std::map<uint64_t, ResourceRegistry> resources;
		std::list<RenderPassRef> renderPasses;

	public:
		FrameGraph(std::vector<DX12RenderPassRef> setupRenderPasses);

		FrameGraph(const FrameGraph &) = delete;
		FrameGraph & operator=(const FrameGraph &) = delete;

		virtual std::vector<RenderPassRef> QueryDanglingRenderPasses() override;

		virtual void EraseRenderPasses(std::vector<RenderPassRef> rps) override;

		virtual std::vector<RenderPassRef> QueryCompleteRenderPasses() override;
	};

	using DX12FrameGraph = Egg::Graphics::DX12::FrameGraph;
	using DX12FrameGraphRef = std::shared_ptr<DX12FrameGraph>;

	class FrameGraphBuilder : public Egg::FrameGraphBuilder {
		std::vector<DX12RenderPassRef> renderPasses;
		IResourceContext * resourceContext;

	public:
		FrameGraphBuilder(IResourceContext * resourceContext) : renderPasses{}, resourceContext{ resourceContext } {

		}

		virtual void CreateRenderPass(const std::string & name,
			SetupCallback setupFunction,
			RenderCallback renderFunction) override {
			renderPasses.emplace_back(std::make_shared<DX12RenderPass>(name, setupFunction, renderFunction));
		}

		virtual FrameGraphRef Build() override {
			for(auto & renderPass : renderPasses) {
				resourceContext->SetRenderPass(renderPass);
				renderPass->Setup(resourceContext);
			}
			resourceContext->ClearRenderPass();

			return std::make_shared<DX12FrameGraph>(std::move(renderPasses));
		}
	};

	using DX12FrameGraphBuilder = Egg::Graphics::DX12::FrameGraphBuilder;
	using DX12FrameGraphBuilderRef = std::shared_ptr<DX12FrameGraphBuilder>;
}
