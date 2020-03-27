#include "DX12StreamOutputBuilder.h"

namespace Netcode::Graphics::DX12 {

	StreamOutputBuilder::StreamOutputBuilder(DX12StreamOutputLibraryRef streamOutputLib) :
		declarations{}, strides{}, rasterizedStream{ D3D12_SO_NO_RASTERIZED_STREAM }, streamOutputLibrary{ streamOutputLib } {

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
			DX12StreamOutputRef so = std::make_shared<DX12StreamOutput>(soDesc, std::move(declarations), std::move(strides));

			streamOutputLibrary->Insert(so);

			return so;
		}

		return candidate;
	}

}

