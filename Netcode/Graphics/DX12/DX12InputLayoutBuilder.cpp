#include "DX12InputLayoutBuilder.h"

namespace Netcode::Graphics::DX12 {

	InputLayoutBuilder::InputLayoutBuilder(Memory::ObjectAllocator allocator, Ref<DX12::InputLayoutLibrary> libRef) : inputElements{ BuilderAllocator<D3D12_INPUT_ELEMENT_DESC>{ allocator } }, inputLayoutLibrary{ libRef } {
		inputElements.reserve(8);
	}
	
	void InputLayoutBuilder::AddInputElement(const char * semanticName, DXGI_FORMAT format) {
		AddInputElement(semanticName, 0, format, D3D12_APPEND_ALIGNED_ELEMENT);
	}

	void InputLayoutBuilder::AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format) {
		AddInputElement(semanticName, semanticIndex, format, D3D12_APPEND_ALIGNED_ELEMENT);
	}

	void InputLayoutBuilder::AddInputElement(const char * semanticName, DXGI_FORMAT format, unsigned int byteOffset) {
		AddInputElement(semanticName, 0, format, byteOffset);
	}

	void InputLayoutBuilder::AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) {
		D3D12_INPUT_ELEMENT_DESC element = { };
		element.AlignedByteOffset = byteOffset;
		element.Format = format;
		element.InputSlot = 0;
		element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		element.SemanticName = semanticName;
		element.SemanticIndex = semanticIndex;
		element.InstanceDataStepRate = 0;
		inputElements.push_back(element);
	}

	Ref<Netcode::InputLayout> InputLayoutBuilder::Build() {
		ASSERT(inputElements.size() > 0, "Input elements not supplied");

		Ref<Netcode::InputLayout> rf = inputLayoutLibrary->GetInputLayout(inputElements);

		if(rf) {
			return rf;
		}

		return inputLayoutLibrary->Insert(std::move(inputElements));
	}

}

