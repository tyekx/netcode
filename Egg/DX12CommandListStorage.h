#pragma once

#include "DX12Common.h"
#include <list>
#include <functional>
#include <algorithm>

namespace Egg::Graphics::DX12 {

	class CommandList {
	public:
		using CommandListT = ID3D12GraphicsCommandList3;
		using DestroyFuncT = std::function<void(D3D12_COMMAND_LIST_TYPE, com_ptr<CommandListT>, com_ptr<ID3D12CommandAllocator>)>;

	private:
		D3D12_COMMAND_LIST_TYPE type;
		com_ptr<CommandListT> commandList;
		com_ptr<ID3D12CommandAllocator> commandAllocator;
		DestroyFuncT onDestroy;

	public:

		CommandList(D3D12_COMMAND_LIST_TYPE type, com_ptr<CommandListT> cl, com_ptr<ID3D12CommandAllocator> ca, DestroyFuncT onDestroy) :
			type{ type }, commandList { cl }, commandAllocator{ ca }, onDestroy{ onDestroy } {
			
		}

		~CommandList() noexcept {
			DX_API("Failed to reset command allocator")
				commandAllocator->Reset();

			DX_API("Failed to reset command list")
				commandList->Reset(commandAllocator.Get(), nullptr);

			onDestroy(type, std::move(commandList), std::move(commandAllocator));
		}
	};

	using CommandListRef = std::shared_ptr<CommandList>;

	class CommandListStorage {
		com_ptr<ID3D12Device> device;

		struct CLCA {
			D3D12_COMMAND_LIST_TYPE type;
			com_ptr<ID3D12GraphicsCommandList3> commandList;
			com_ptr<ID3D12CommandAllocator> commandAllocator;

			CLCA(ID3D12Device * device, D3D12_COMMAND_LIST_TYPE type) : type{ type }, commandList{ nullptr }, commandAllocator{ nullptr } {
				DX_API("Failed to create command allocator")
					device->CreateCommandAllocator(type, IID_PPV_ARGS(commandAllocator.GetAddressOf()));

				DX_API("Failed to create command list")
					device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));
			}

			CLCA(D3D12_COMMAND_LIST_TYPE type, com_ptr<ID3D12GraphicsCommandList3> cl, com_ptr<ID3D12CommandAllocator> ca) : type{ type }, commandList{ std::move(cl) }, commandAllocator{ std::move(ca) } {

			}
		};

		std::list<CLCA> clearPairs;

		template<D3D12_COMMAND_LIST_TYPE TYPE>
		CommandListRef GetCommandList() {
			auto it = std::find_if(std::begin(clearPairs), std::end(clearPairs), [](const CLCA & clca) -> bool {
				return clca.type == TYPE;
			});

			if(it != std::end(clearPairs)) {
				CommandListRef clr = std::make_shared<CommandList>(it->type, std::move(it->commandList), std::move(it->commandAllocator),
																   [this](D3D12_COMMAND_LIST_TYPE type, com_ptr<ID3D12GraphicsCommandList3> cl, com_ptr<ID3D12CommandAllocator> ca) -> void {
					clearPairs.emplace_back(type, std::move(cl), std::move(ca));
				});
				clearPairs.erase(it);
				return clr;
			}

			CLCA clca{ device.Get(), TYPE };

			CommandListRef clr = std::make_shared<CommandList>(clca.type, std::move(clca.commandList), std::move(clca.commandAllocator),
															   [this](D3D12_COMMAND_LIST_TYPE type, com_ptr<ID3D12GraphicsCommandList3> cl, com_ptr<ID3D12CommandAllocator> ca) -> void {
				clearPairs.emplace_back(type, std::move(cl), std::move(ca));
			});

			return clr;
		}

	public:
		void SetDevice(com_ptr<ID3D12Device> dev) {
			device = std::move(dev);
		}

		CommandListRef GetDirect() {
			return GetCommandList<D3D12_COMMAND_LIST_TYPE_DIRECT>();
		}

		CommandListRef GetCompute() {
			return GetCommandList<D3D12_COMMAND_LIST_TYPE_COMPUTE>();
		}

		CommandListRef GetCopy() {
			return GetCommandList<D3D12_COMMAND_LIST_TYPE_COPY>();
		}
	};

}
