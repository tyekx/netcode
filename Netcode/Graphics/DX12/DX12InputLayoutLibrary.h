#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class InputLayoutLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<InputLayout>> inputLayouts;

	public:
		InputLayoutLibrary(Memory::ObjectAllocator allocator);

		Ref<InputLayout> Insert(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> inputElements);

		Ref<InputLayout> GetInputLayout(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & desc);
	};

}
