#include "DX12InputLayoutLibrary.h"
#include <iostream>

namespace Netcode::Graphics::DX12 {


	InputLayoutLibrary::InputLayoutLibrary(Memory::ObjectAllocator allocator) : inputLayouts{ 
		BuilderAllocator<DX12InputLayoutRef>{ allocator } }, objectAllocator{ allocator } {
		inputLayouts.reserve(8);
	}

	InputLayoutRef InputLayoutLibrary::Insert(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> inputElements) {
		auto sharedPtr = std::make_shared<DX12InputLayout>(std::move(inputElements));

		inputLayouts.push_back(sharedPtr);

		return sharedPtr;
	}

	InputLayoutRef InputLayoutLibrary::GetInputLayout(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & desc) {
		for(const auto & i : inputLayouts) {
			if(*i == desc) {
				return i;
			}
		}
		return nullptr;
	}

}

