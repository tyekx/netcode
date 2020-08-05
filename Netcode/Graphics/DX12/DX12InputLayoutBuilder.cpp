#include "DX12InputLayoutBuilder.h"
#include <Netcode/Common.h>
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	InputLayoutBuilderImpl::InputLayoutBuilderImpl(Memory::ObjectAllocator allocator, Ref<DX12::InputLayoutLibrary> libRef) : inputElements{ BuilderAllocator<D3D12_INPUT_ELEMENT_DESC>{ allocator } }, inputLayoutLibrary{ libRef } {
		inputElements.reserve(8);
	}
	
	void InputLayoutBuilderImpl::AddInputElement(const char * semanticName, DXGI_FORMAT format) {
		AddInputElement(semanticName, 0, format, D3D12_APPEND_ALIGNED_ELEMENT);
	}

	void InputLayoutBuilderImpl::AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format) {
		AddInputElement(semanticName, semanticIndex, format, D3D12_APPEND_ALIGNED_ELEMENT);
	}

	void InputLayoutBuilderImpl::AddInputElement(const char * semanticName, DXGI_FORMAT format, unsigned int byteOffset) {
		AddInputElement(semanticName, 0, format, byteOffset);
	}

	void InputLayoutBuilderImpl::AddInputElement(const char * semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) {
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

	Ref<InputLayout> InputLayoutBuilderImpl::Build() {
		ASSERT(inputElements.size() > 0, "Input elements not supplied");

		Ref<InputLayout> rf = inputLayoutLibrary->GetInputLayout(inputElements);

		if(rf) {
			return rf;
		}

		return inputLayoutLibrary->Insert(std::move(inputElements));
	}

}

