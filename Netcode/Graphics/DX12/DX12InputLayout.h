#pragma once

#include "../../HandleTypes.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class InputLayout : public Netcode::InputLayout {
		D3D12_INPUT_LAYOUT_DESC inputLayout;
		std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
	public:
		InputLayout(const D3D12_INPUT_LAYOUT_DESC & il, std::vector<D3D12_INPUT_ELEMENT_DESC> && elements);

		const D3D12_INPUT_LAYOUT_DESC & GetNativeInputLayout() const;

		bool operator==(const std::vector<D3D12_INPUT_ELEMENT_DESC> & rhs) const;
	};

	using DX12InputLayout = Netcode::Graphics::DX12::InputLayout;
	using DX12InputLayoutRef = std::shared_ptr<DX12InputLayout>;

}
