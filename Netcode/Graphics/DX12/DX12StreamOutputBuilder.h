#pragma once

#include "DX12StreamOutput.h"
#include "DX12StreamOutputLibrary.h"

namespace Netcode::Graphics::DX12 {

	class StreamOutputBuilder : public Netcode::StreamOutputBuilder {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<D3D12_SO_DECLARATION_ENTRY> declarations;
		BuilderContainer<UINT> strides;
		UINT rasterizedStream;
		Ref<DX12::StreamOutputLibrary> streamOutputLibrary;
	public:

		StreamOutputBuilder(Memory::ObjectAllocator alloc, Ref<DX12::StreamOutputLibrary> streamOutputLib);

		virtual void AddStreamOutputEntry(const char * semanticName, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) override;

		virtual void AddStride(uint32_t stride) override;

		virtual void SetRasterizedStream(uint32_t stream) override;

		virtual void AddStreamOutputEntry(const char * semanticName, uint32_t semanticIndex, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) override;

		virtual Ref<Netcode::StreamOutput> Build() override;
	};

}
