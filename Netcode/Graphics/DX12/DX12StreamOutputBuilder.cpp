#include "DX12StreamOutputBuilder.h"
#include "DX12Includes.h"
#include "DX12StreamOutput.h"
#include "DX12StreamOutputLibrary.h"

namespace Netcode::Graphics::DX12 {

	StreamOutputBuilderImpl::StreamOutputBuilderImpl(Memory::ObjectAllocator alloc, Ref<DX12::StreamOutputLibrary> streamOutputLib) :
		objectAllocator{ alloc },
		declarations{ BuilderAllocator<D3D12_SO_DECLARATION_ENTRY>{alloc} },
		strides{ BuilderAllocator<uint32_t>{alloc} },
		rasterizedStream{ D3D12_SO_NO_RASTERIZED_STREAM },
		streamOutputLibrary{ std::move(streamOutputLib) } {
		declarations.reserve(4);
		strides.reserve(8);
	}

	void StreamOutputBuilderImpl::AddStreamOutputEntry(const char * semanticName, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) {
		AddStreamOutputEntry(semanticName, 0, componentCount, outputSlot, startComponent, stream);
	}
	void StreamOutputBuilderImpl::AddStride(uint32_t stride) {
		strides.push_back(stride);
	}
	void StreamOutputBuilderImpl::SetRasterizedStream(uint32_t stream) {
		rasterizedStream = stream;
	}

	void StreamOutputBuilderImpl::AddStreamOutputEntry(const char * semanticName, uint32_t semanticIndex, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) {
		D3D12_SO_DECLARATION_ENTRY entry;
		entry.ComponentCount = componentCount;
		entry.SemanticIndex = semanticIndex;
		entry.OutputSlot = outputSlot;
		entry.SemanticName = semanticName;
		entry.StartComponent = startComponent;
		entry.Stream = stream;
		declarations.push_back(entry);
	}

	Ref<Netcode::StreamOutput> StreamOutputBuilderImpl::Build() {
		D3D12_STREAM_OUTPUT_DESC soDesc;
		soDesc.NumEntries = static_cast<uint32_t>(declarations.size());
		soDesc.pSODeclaration = declarations.data();
		soDesc.NumStrides = static_cast<uint32_t>(strides.size());
		soDesc.pBufferStrides = strides.data();
		soDesc.RasterizedStream = rasterizedStream;

		auto candidate = streamOutputLibrary->GetStreamOutput(soDesc);

		if(candidate == nullptr) {
			Ref<StreamOutputImpl> so = objectAllocator.MakeShared<StreamOutputImpl>(soDesc, std::move(declarations), std::move(strides));

			streamOutputLibrary->Insert(so);

			return so;
		}

		return candidate;
	}

}

