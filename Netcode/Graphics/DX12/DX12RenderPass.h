#pragma once

#include <Netcode/HandleDecl.h>
#include <Netcode/Graphics/FrameGraph.h>
#include <functional>

namespace Netcode::Graphics::DX12 {

	class RenderPassImpl : public Netcode::RenderPass {
	protected:
		uint64_t read[32];
		uint64_t written[32];
		uint32_t numRead;
		uint32_t numWritten;
		RenderPassType type;

	public:
		std::string name;
		SetupCallback Setup;
		RenderCallback Render;

		RenderPassImpl(const std::string & name, SetupCallback setup, RenderCallback render);

		virtual void Type(Netcode::RenderPassType value) override;
		virtual RenderPassType Type() const override;

		virtual void ReadsResource(uint64_t resource) override;
		virtual void WritesResource(uint64_t resource) override;

		virtual ArrayView<uint64_t> GetReadResources() override;
		virtual ArrayView<uint64_t> GetWrittenResources() override;
	};

}
