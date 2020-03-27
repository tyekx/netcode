#include "DX12InputLayoutLibrary.h"

namespace Egg::Graphics::DX12 {

	InputLayoutRef InputLayoutLibrary::Insert(std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements) {
		D3D12_INPUT_LAYOUT_DESC ld;
		ld.NumElements = static_cast<UINT>(inputElements.size());
		ld.pInputElementDescs = inputElements.data();

		auto sharedPtr = std::make_shared<DX12InputLayout>(ld, std::move(inputElements));

		inputLayouts.push_back(sharedPtr);

		return sharedPtr;
	}

	InputLayoutRef InputLayoutLibrary::GetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC> & desc) {
		for(const auto & i : inputLayouts) {
			if(*i == desc) {
				return i;
			}
		}
		return nullptr;
	}

}

