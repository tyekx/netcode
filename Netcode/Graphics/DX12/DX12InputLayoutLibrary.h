#pragma once

#include "DX12InputLayout.h"

namespace Netcode::Graphics::DX12 {
	class InputLayoutLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<DX12InputLayoutRef> inputLayouts;

	public:
		InputLayoutLibrary(Memory::ObjectAllocator allocator);

		InputLayoutRef Insert(BuilderContainer<D3D12_INPUT_ELEMENT_DESC> inputElements);

		InputLayoutRef GetInputLayout(const BuilderContainer<D3D12_INPUT_ELEMENT_DESC> & desc);
	};

	using DX12InputLayoutLibrary = Netcode::Graphics::DX12::InputLayoutLibrary;
	using DX12InputLayoutLibraryRef = std::shared_ptr<DX12InputLayoutLibrary>;
}
