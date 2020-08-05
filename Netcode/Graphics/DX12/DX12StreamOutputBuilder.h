#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12Common.h"
#include "DX12Decl.h"

namespace Netcode::Graphics::DX12 {

	class StreamOutputLibrary;

	class StreamOutputBuilderImpl : public Netcode::StreamOutputBuilder {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<D3D12_SO_DECLARATION_ENTRY> declarations;
		BuilderContainer<uint32_t> strides;
		uint32_t rasterizedStream;
		Ref<StreamOutputLibrary> streamOutputLibrary;
	public:

		StreamOutputBuilderImpl(Memory::ObjectAllocator alloc, Ref<StreamOutputLibrary> streamOutputLib);

		virtual void AddStreamOutputEntry(const char * semanticName, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) override;

		virtual void AddStride(uint32_t stride) override;

		virtual void SetRasterizedStream(uint32_t stream) override;

		virtual void AddStreamOutputEntry(const char * semanticName, uint32_t semanticIndex, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) override;

		virtual Ref<Netcode::StreamOutput> Build() override;
	};

}
