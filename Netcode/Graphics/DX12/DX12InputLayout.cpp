#include "../../FancyIterators.hpp"
#include "DX12InputLayout.h"

namespace Egg::Graphics::DX12 {

	InputLayout::InputLayout(const D3D12_INPUT_LAYOUT_DESC & il, std::vector<D3D12_INPUT_ELEMENT_DESC> && elements) : inputLayout{ il }, elements{ std::move(elements) } { }

	const D3D12_INPUT_LAYOUT_DESC & InputLayout::GetNativeInputLayout() const {
		return inputLayout;
	}

	// @TODO: handle the automatically aligned version
	bool InputLayout::operator==(const std::vector<D3D12_INPUT_ELEMENT_DESC> & rhs) const {
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

