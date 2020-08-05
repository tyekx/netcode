#include "DX12InputLayout.h"
#include <Netcode/FancyIterators.hpp>

namespace Netcode::Graphics::DX12 {

	InputLayoutImpl::InputLayoutImpl(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> elements) : inputLayout{ }, elements{ std::move(elements) } {
		inputLayout.NumElements = this->elements.size();
		inputLayout.pInputElementDescs = this->elements.data();
	}

	const D3D12_INPUT_LAYOUT_DESC & InputLayoutImpl::GetNativeInputLayout() const {
		return inputLayout;
	}

	bool InputLayoutImpl::operator==(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & rhs) const {
		if(elements.size() != rhs.size()) {
			return false;
		}

		auto zipped = ZipConst(elements, rhs);

		for(const auto [left, right] : zipped) {
			if(memcmp(&left, &right, sizeof(D3D12_INPUT_ELEMENT_DESC)) != 0) {
				return false;
			}
		}

		return true;
	}
}

