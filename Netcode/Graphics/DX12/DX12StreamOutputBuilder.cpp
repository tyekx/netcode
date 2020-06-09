#include "DX12StreamOutputBuilder.h"

namespace Netcode::Graphics::DX12 {

	StreamOutputBuilder::StreamOutputBuilder(Memory::ObjectAllocator alloc, DX12StreamOutputLibraryRef streamOutputLib) :
		objectAllocator{ alloc },
		declarations{ BuilderAllocator<D3D12_SO_DECLARATION_ENTRY>{alloc} },
		strides{ BuilderAllocator<UINT>{alloc} },
		rasterizedStream{ D3D12_SO_NO_RASTERIZED_STREAM },
		streamOutputLibrary{ streamOutputLib } {
		declarations.reserve(4);
		strides.reserve(8);
	}

	void StreamOutputBuilder::AddStreamOutputEntry(const char * semanticName, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) {
		AddStreamOutputEntry(semanticName, 0, componentCount, outputSlot, startComponent, stream);
	}
	void StreamOutputBuilder::AddStride(uint32_t stride) {
		strides.push_back(stride);
	}
	void StreamOutputBuilder::SetRasterizedStream(uint32_t stream) {
		rasterizedStream = stream;
	}

	void StreamOutputBuilder::AddStreamOutputEntry(const char * semanticName, uint32_t semanticIndex, uint8_t componentCount, uint8_t outputSlot, uint8_t startComponent, uint32_t stream) {
		D3D12_SO_DECLARATION_ENTRY entry;
		entry.ComponentCount = componentCount;
		entry.SemanticIndex = semanticIndex;
		entry.OutputSlot = outputSlot;
		entry.SemanticName = semanticName;
		entry.StartComponent = startComponent;
		entry.Stream = stream;
		declarations.push_back(entry);
	}

	StreamOutputRef StreamOutputBuilder::Build() {
		D3D12_STREAM_OUTPUT_DESC soDesc;
		soDesc.NumEntries = static_cast<UINT>(declarations.size());
		soDesc.pSODeclaration = declarations.data();
		soDesc.NumStrides = static_cast<UINT>(strides.size());
		soDesc.pBufferStrides = strides.data();
		soDesc.RasterizedStream = rasterizedStream;

		auto candidate = streamOutputLibrary->GetStreamOutput(soDesc);

		if(candidate == nullptr) {
			DX12StreamOutputRef so = objectAllocator.MakeShared<DX12StreamOutput>(soDesc, std::move(declarations), std::move(strides));

			streamOutputLibrary->Insert(so);

			return so;
		}

		return candidate;
	}

}

