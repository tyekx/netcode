#pragma once

#include "DX12StreamOutput.h"
#include "DX12StreamOutputLibrary.h"

namespace Egg::Graphics::DX12 {

	class StreamOutputBuilder : public Egg::StreamOutputBuilder {
		std::vector<D3D12_SO_DECLARATION_ENTRY> declarations;
		std::vector<UINT> strides;
		UINT rasterizedStream;
		DX12StreamOutputLibraryRef streamOutputLibrary;
	public:

		StreamOutputBuilder(DX12StreamOutputLibraryRef streamOutputLib);

		virtual void AddStreamOutputEntry(const char * semanticName, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) override;

		virtual void AddStride(uint32_t stride) override;

		virtual void SetRasterizedStream(uint32_t stream) override;

		virtual void AddStreamOutputEntry(const char * semanticName, uint32_t semanticIndex, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) override;

		virtual StreamOutputRef Build() override;
	};

	using DX12StreamOutputBuilder = Egg::Graphics::DX12::StreamOutputBuilder;
	using DX12StreamOutputBuilderRef = std::shared_ptr<Egg::Graphics::DX12::StreamOutputBuilder>;

}
