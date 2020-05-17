#include "DX12CommandListPool.h"
#include <memory>
#include <functional>

namespace Netcode::Graphics::DX12 {

	com_ptr<ID3D12CommandAllocator> CommandListPool::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) {
		com_ptr<ID3D12CommandAllocator> alloc;

		uint32_t typeInt = static_cast<uint32_t>(type);

		DX_API("Failed to create command allocator")
			device->CreateCommandAllocator(type, IID_PPV_ARGS(alloc.GetAddressOf()));

		DX_API("Failed to set debug name")
			alloc->SetName(IndexedDebugName(CommandListTypeToString(type), numAlloc[typeInt]).c_str());

		numAlloc[typeInt] += 1;

		return alloc;
	}

	com_ptr<CommandListType> CommandListPool::CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator * ca) {
		com_ptr<CommandListType> cl;

		uint32_t typeInt = static_cast<uint32_t>(type);

		DX_API("Failed to create command list")
			device->CreateCommandList(0, type, ca, nullptr, IID_PPV_ARGS(cl.GetAddressOf()));
		
		DX_API("Failed to set debug name")
			cl->SetName(IndexedDebugName(CommandListTypeToString(type), numCls[typeInt]).c_str());

		numCls[typeInt] += 1;

		return cl;
	}

	com_ptr<ID3D12CommandAllocator> CommandListPool::GetAllocatorImpl(D3D12_COMMAND_LIST_TYPE type, std::vector<com_ptr<ID3D12CommandAllocator>> & allocStorage) {
		if(allocStorage.empty()) {
			return CreateCommandAllocator(type);
		} else {
			auto ptr = allocStorage.back();
			allocStorage.pop_back();
			return ptr;
		}
	}

	CommandListPool::PairedCommandList CommandListPool::GetPairImpl(D3D12_COMMAND_LIST_TYPE type, std::vector<PairedCommandList> & pairStorage) {
		PairedCommandList pcl;
		if(pairStorage.empty()) {
			pcl.allocator = GetAllocator(type);
			pcl.gcl = CreateCommandList(type, pcl.allocator.Get());
		} else {
			pcl = pairStorage.back();
			pairStorage.pop_back();
		}
		return pcl;
	}

	com_ptr<ID3D12CommandAllocator> CommandListPool::GetAllocator(D3D12_COMMAND_LIST_TYPE type) {
		switch(type) {
			case D3D12_COMMAND_LIST_TYPE_DIRECT: return GetAllocatorImpl(type, directAllocators);
			case D3D12_COMMAND_LIST_TYPE_COMPUTE: return GetAllocatorImpl(type, computeAllocators);
			case D3D12_COMMAND_LIST_TYPE_COPY: return GetAllocatorImpl(type, copyAllocators);
			default: return nullptr;
		}
	}

	void CommandListPool::StoreAllocator(D3D12_COMMAND_LIST_TYPE type, com_ptr<ID3D12CommandAllocator> alloc) {
		switch(type) {
			case D3D12_COMMAND_LIST_TYPE_DIRECT: directAllocators.emplace_back(std::move(alloc)); break;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE: computeAllocators.emplace_back(std::move(alloc)); break;
			case D3D12_COMMAND_LIST_TYPE_COPY: copyAllocators.emplace_back(std::move(alloc)); break;
			default: break;
		}
	}

	CommandListPool::PairedCommandList CommandListPool::GetPair(D3D12_COMMAND_LIST_TYPE type) {
		switch(type) {
			case D3D12_COMMAND_LIST_TYPE_DIRECT: return GetPairImpl(type, directPairs);
			case D3D12_COMMAND_LIST_TYPE_COMPUTE: return GetPairImpl(type, computePairs);
			case D3D12_COMMAND_LIST_TYPE_COPY: return GetPairImpl(type, copyPairs);
			default: return PairedCommandList{ };
		}
	}

	void CommandListPool::StorePair(D3D12_COMMAND_LIST_TYPE type, PairedCommandList pcl) {
		switch(type) {
			case D3D12_COMMAND_LIST_TYPE_DIRECT: directPairs.emplace_back(std::move(pcl)); break;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE: computePairs.emplace_back(std::move(pcl)); break;
			case D3D12_COMMAND_LIST_TYPE_COPY: copyPairs.emplace_back(std::move(pcl)); break;
			default: break;
		}
	}

	void CommandListPool::ReturnAllocator(ID3D12CommandAllocator * alloc, D3D12_COMMAND_LIST_TYPE type) {
		com_ptr<ID3D12CommandAllocator> managedAlloc{ nullptr };
		managedAlloc.Attach(alloc);

		DX_API("Failed to reset command allocator")
			managedAlloc->Reset();

		StoreAllocator(type, std::move(managedAlloc));
	}

	void CommandListPool::ReturnCommandList(CommandListType * gcl, D3D12_COMMAND_LIST_TYPE type) {
		PairedCommandList pcl;
		pcl.gcl.Attach(gcl);
		pcl.allocator = GetAllocator(type);

		DX_API("Failed to reset command list")
			pcl.gcl->Reset(pcl.allocator.Get(), nullptr);

		StorePair(type, std::move(pcl));
	}

	CommandList CommandListPool::CreateBindings(D3D12_COMMAND_LIST_TYPE type, PairedCommandList pcl) {
		ID3D12CommandAllocator * allocator = pcl.allocator.Detach();
		CommandListType * gcl = pcl.gcl.Detach();

		return CommandList(
			std::shared_ptr<ID3D12CommandAllocator>(allocator, std::bind(&CommandListPool::ReturnAllocator, this, std::placeholders::_1, type)),
			std::shared_ptr<CommandListType>(gcl, std::bind(&CommandListPool::ReturnCommandList, this, std::placeholders::_1, type))
		);
	}

	CommandList CommandListPool::GetCommandList(D3D12_COMMAND_LIST_TYPE type) {
		PairedCommandList pcl = GetPair(type);
		return CreateBindings(type, std::move(pcl));
	}

	CommandListPool::CommandListPool(com_ptr<ID3D12Device> device) :
		numCls{},
		numAlloc{},
		device{ std::move(device) },
		directAllocators{}, computeAllocators{}, copyAllocators{},
		directPairs{}, computePairs{}, copyPairs{} {

		constexpr uint32_t NUM_DIRECT_LISTS = 16;
		constexpr uint32_t NUM_COMPUTE_LISTS = 4;
		constexpr uint32_t NUM_COPY_LISTS = 4;

		{
			std::vector<PairedCommandList> gPairs;
			directAllocators.reserve(NUM_DIRECT_LISTS * 2);
			gPairs.reserve(NUM_DIRECT_LISTS * 2);
			for(uint32_t i = 0; i < NUM_DIRECT_LISTS; ++i) {
				directAllocators.emplace_back(CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT));
				directAllocators.emplace_back(CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT));
				gPairs.emplace_back(GetPair(D3D12_COMMAND_LIST_TYPE_DIRECT));
			}
			directPairs = std::move(gPairs);
		}

		{
			std::vector<PairedCommandList> cPairs;
			computeAllocators.reserve(NUM_COMPUTE_LISTS * 2);
			cPairs.reserve(NUM_COMPUTE_LISTS * 2);
			for(uint32_t i = 0; i < NUM_COMPUTE_LISTS; ++i) {
				computeAllocators.emplace_back(CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE));
				computeAllocators.emplace_back(CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE));
				cPairs.emplace_back(GetPair(D3D12_COMMAND_LIST_TYPE_COMPUTE));
			}
			computePairs = std::move(cPairs);
		}

		{
			std::vector<PairedCommandList> cPairs;
			copyAllocators.reserve(NUM_COPY_LISTS * 2);
			cPairs.reserve(NUM_COPY_LISTS * 2);
			for(uint32_t i = 0; i < NUM_COPY_LISTS; ++i) {
				copyAllocators.emplace_back(CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY));
				copyAllocators.emplace_back(CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY));
				cPairs.emplace_back(GetPair(D3D12_COMMAND_LIST_TYPE_COPY));
			}
			copyPairs = std::move(cPairs);
		}
	}

	CommandList CommandListPool::GetDirect() {
		return GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	CommandList CommandListPool::GetCopy() {
		return GetCommandList(D3D12_COMMAND_LIST_TYPE_COPY);
	}

	CommandList CommandListPool::GetCompute() {
		return GetCommandList(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}

}
