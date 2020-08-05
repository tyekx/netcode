#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Decl.h"
#include <memory>

namespace Netcode::Graphics::DX12 {

	using CommandListType = ID3D12GraphicsCommandList3;

	class CommandList {
		Ref<CommandListType> boundCl;
		Ref<ID3D12CommandAllocator> boundAllocator;

	public:
		CommandList(Ref<ID3D12CommandAllocator> alloc, Ref<CommandListType> cl) : boundCl{ std::move(cl) },
			boundAllocator{ std::move(alloc) } { }

		CommandListType * GetCommandList() const {
			return boundCl.get();
		}

		void ResetCommandList() {
			boundCl.reset();
		}

		void ResetCommandAllocator() {
			boundAllocator.reset();
		}
	};

}
