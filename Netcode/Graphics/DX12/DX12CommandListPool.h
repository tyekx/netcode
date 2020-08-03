#pragma once

#include "DX12Common.h"
#include <vector>
#include <string>

#include "DX12CommandList.h"

namespace Netcode::Graphics::DX12 {

	class CommandListPool {
		uint32_t numCls[4];
		uint32_t numAlloc[4];

		com_ptr<ID3D12Device> device;

		struct PairedCommandList {
			com_ptr<ID3D12CommandAllocator> allocator;
			com_ptr<CommandListType> gcl;
		};

		std::vector<com_ptr<ID3D12CommandAllocator>> directAllocators;
		std::vector<com_ptr<ID3D12CommandAllocator>> computeAllocators;
		std::vector<com_ptr<ID3D12CommandAllocator>> copyAllocators;

		std::vector<PairedCommandList> directPairs;
		std::vector<PairedCommandList> computePairs;
		std::vector<PairedCommandList> copyPairs;

		com_ptr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type);

		com_ptr<CommandListType> CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator * ca);

		com_ptr<ID3D12CommandAllocator> GetAllocatorImpl(D3D12_COMMAND_LIST_TYPE type, std::vector<com_ptr<ID3D12CommandAllocator>> & allocStorage);

		PairedCommandList GetPairImpl(D3D12_COMMAND_LIST_TYPE type, std::vector<PairedCommandList> & pairStorage);

		com_ptr<ID3D12CommandAllocator> GetAllocator(D3D12_COMMAND_LIST_TYPE type);

		void StoreAllocator(D3D12_COMMAND_LIST_TYPE type, com_ptr<ID3D12CommandAllocator> alloc);

		PairedCommandList GetPair(D3D12_COMMAND_LIST_TYPE type);

		void StorePair(D3D12_COMMAND_LIST_TYPE type, PairedCommandList pcl);

		void ReturnAllocator(ID3D12CommandAllocator * alloc, D3D12_COMMAND_LIST_TYPE type);

		void ReturnCommandList(CommandListType * gcl, D3D12_COMMAND_LIST_TYPE type);

		CommandList CreateBindings(D3D12_COMMAND_LIST_TYPE type, PairedCommandList pcl);

		CommandList GetCommandList(D3D12_COMMAND_LIST_TYPE type);

	public:

		CommandListPool(com_ptr<ID3D12Device> device);

		CommandList GetDirect();

		CommandList GetCopy();

		CommandList GetCompute();

	};

	using DX12CommandListPool = Netcode::Graphics::DX12::CommandListPool;
	using DX12CommandListPoolRef = std::shared_ptr<DX12CommandListPool>;

}
