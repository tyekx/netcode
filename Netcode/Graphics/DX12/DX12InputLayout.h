#pragma once

#include "../../HandleTypes.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class InputLayout : public Netcode::InputLayout {
		D3D12_INPUT_LAYOUT_DESC inputLayout;
		BuilderContainer<D3D12_INPUT_ELEMENT_DESC> elements;
	public:
		InputLayout(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> elements);

		const D3D12_INPUT_LAYOUT_DESC & GetNativeInputLayout() const;

		bool operator==(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & rhs) const;
	};

}
