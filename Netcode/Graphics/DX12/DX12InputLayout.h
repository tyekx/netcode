#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12Common.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	class InputLayoutImpl : public InputLayout {
		D3D12_INPUT_LAYOUT_DESC inputLayout;
		BuilderContainer<D3D12_INPUT_ELEMENT_DESC> elements;
	public:
		InputLayoutImpl(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> elements);

		const D3D12_INPUT_LAYOUT_DESC & GetNativeInputLayout() const;

		bool operator==(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & rhs) const;
	};

}
