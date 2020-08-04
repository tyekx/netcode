#pragma once

#include "DX12InputLayout.h"

namespace Netcode::Graphics::DX12 {
	class InputLayoutLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<DX12::InputLayout>> inputLayouts;

	public:
		InputLayoutLibrary(Memory::ObjectAllocator allocator);

		Ref<DX12::InputLayout> Insert(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> inputElements);

		Ref<DX12::InputLayout> GetInputLayout(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & desc);
	};

}
