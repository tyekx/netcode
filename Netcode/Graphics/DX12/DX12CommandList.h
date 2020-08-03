#pragma once

#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	using CommandListType = ID3D12GraphicsCommandList3;

	class CommandList {
		// dont change the order!
		std::shared_ptr<CommandListType> boundCl;
		std::shared_ptr<ID3D12CommandAllocator> boundAllocator;

	public:
		CommandList(std::shared_ptr<ID3D12CommandAllocator> alloc, std::shared_ptr<CommandListType> cl) : boundCl{ std::move(cl) },
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
