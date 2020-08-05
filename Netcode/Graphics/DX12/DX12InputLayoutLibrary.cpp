#include "DX12InputLayoutLibrary.h"
#include "DX12InputLayout.h"

namespace Netcode::Graphics::DX12 {


	InputLayoutLibrary::InputLayoutLibrary(Memory::ObjectAllocator allocator) :objectAllocator{ allocator }, inputLayouts{
		BuilderAllocator<Ref<InputLayout>>{ allocator } } {
		inputLayouts.reserve(8);
	}

	Ref<InputLayout> InputLayoutLibrary::Insert(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> inputElements) {
		auto sharedPtr = std::make_shared<InputLayoutImpl>(std::move(inputElements));

		inputLayouts.push_back(sharedPtr);

		return sharedPtr;
	}

	Ref<InputLayout> InputLayoutLibrary::GetInputLayout(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & desc) {
		for(const auto & i : inputLayouts) {
			const auto* il = static_cast<const InputLayoutImpl*>(i.get());

			if(*il == desc) {
				return i;
			}
		}
		return nullptr;
	}

}

