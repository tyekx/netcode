#pragma once

#include "DX12InputLayout.h"

namespace Netcode::Graphics::DX12 {
	class InputLayoutLibrary {
		std::vector<DX12InputLayoutRef> inputLayouts;

	public:
		InputLayoutRef Insert(std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements);

		InputLayoutRef GetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC> & desc);
	};

	using DX12InputLayoutLibrary = Netcode::Graphics::DX12::InputLayoutLibrary;
	using DX12InputLayoutLibraryRef = std::shared_ptr<DX12InputLayoutLibrary>;
}
