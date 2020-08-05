#include "DX12RenderPass.h"
#include <NetcodeFoundation/ArrayView.hpp>

namespace Netcode::Graphics::DX12 {

	RenderPassImpl::RenderPassImpl(const std::string & name, SetupCallback setup, RenderCallback render) :
		read{}, written{}, numRead{ 0 }, numWritten{ 0 }, type{ RenderPassType::GRAPHICS }, name{ name }, Setup{ setup }, Render{ render } {

	}

	ArrayView<uint64_t> RenderPassImpl::GetReadResources() {
		return ArrayView<uint64_t>(read, static_cast<size_t>(numRead));
	}

	ArrayView<uint64_t> RenderPassImpl::GetWrittenResources() {
		return ArrayView<uint64_t>(written, static_cast<size_t>(numWritten));
	}

	void RenderPassImpl::Type(Netcode::RenderPassType value) {
		type = value;
	}

	RenderPassType RenderPassImpl::Type() const {
		return type;
	}

	void RenderPassImpl::ReadsResource(uint64_t resource) {
		read[numRead++] = resource;
	}

	void RenderPassImpl::WritesResource(uint64_t resource) {
		written[numWritten++] = resource;
	}

}
