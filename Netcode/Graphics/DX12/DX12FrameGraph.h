#pragma once

#include "../FrameGraph.h"
#include "../../LinearAllocator.h"
#include <NetcodeFoundation/ArrayView.hpp>

namespace Netcode::Graphics::DX12 {

	class RenderPass : public Netcode::RenderPass {
	protected:
		uint64_t read[32];
		uint64_t written[32];
		uint32_t numRead;
		uint32_t numWritten;
		Netcode::RenderPassType type;

	public:
		
		RenderPass(const std::string & name, Netcode::SetupCallback setup, Netcode::RenderCallback render) : Netcode::RenderPass(name, setup, render),
			read{}, written{}, numRead{ 0 }, numWritten{ 0 }, type{ Netcode::RenderPassType::DIRECT } {

		}

		virtual void Type(Netcode::RenderPassType value) override {
			type = value;
		}

		virtual Netcode::RenderPassType Type() const override {
			return type;
		}

		virtual void ReadsResource(uint64_t resource) override {
			read[numRead++] = resource;
		}

		virtual void WritesResource(uint64_t resource) override {
			written[numWritten++] = resource;
		}

		virtual Netcode::ArrayView<uint64_t> GetReadResources() override {
			return Netcode::ArrayView<uint64_t>(read, static_cast<size_t>(numRead));
		}

		virtual Netcode::ArrayView<uint64_t> GetWrittenResources() override {
			return Netcode::ArrayView<uint64_t>(written, static_cast<size_t>(numWritten));
		}
	};

	class FrameGraph : public Netcode::FrameGraph {
	private:
		struct ResourceRegistry {
			uint32_t numReads;
			uint32_t numWrites;

			ResourceRegistry() : numReads{ 0 }, numWrites{ 0 } { }
		};

		std::map<uint64_t, ResourceRegistry> resources;
		std::list<Ref<RenderPass>> renderPasses;

	public:
		FrameGraph(std::vector<Ref<DX12::RenderPass>> setupRenderPasses);

		FrameGraph(const FrameGraph &) = delete;
		FrameGraph & operator=(const FrameGraph &) = delete;

		virtual std::vector<Ref<Netcode::RenderPass>> QueryDanglingRenderPasses() override;

		virtual void EraseRenderPasses(std::vector<Ref<Netcode::RenderPass>> rps) override;

		virtual std::vector<Ref<Netcode::RenderPass>> QueryCompleteRenderPasses() override;

		virtual bool UsingBackbuffer() const override;
	};

	using DX12FrameGraph = Netcode::Graphics::DX12::FrameGraph;
	using DX12FrameGraphRef = std::shared_ptr<DX12FrameGraph>;

	class FrameGraphBuilder : public Netcode::FrameGraphBuilder {
		std::vector<Ref<DX12::RenderPass>> renderPasses;
		std::shared_ptr<IResourceContext> resourceContext;

	public:
		FrameGraphBuilder(std::shared_ptr<IResourceContext> resourceContext) : renderPasses{}, resourceContext{ std::move( resourceContext ) } {

		}

		virtual void CreateRenderPass(const std::string & name,
			SetupCallback setupFunction,
			RenderCallback renderFunction) override {
			renderPasses.emplace_back(std::make_shared<DX12::RenderPass>(name, setupFunction, renderFunction));
		}

		virtual Ref<FrameGraph> Build() override {
			for(auto & renderPass : renderPasses) {
				resourceContext->SetRenderPass(renderPass);
				renderPass->Setup(resourceContext.get());
			}
			resourceContext->ClearRenderPass();

			return std::make_shared<DX12FrameGraph>(std::move(renderPasses));
		}
	};

	using DX12FrameGraphBuilder = Netcode::Graphics::DX12::FrameGraphBuilder;
	using DX12FrameGraphBuilderRef = std::shared_ptr<DX12FrameGraphBuilder>;
}
