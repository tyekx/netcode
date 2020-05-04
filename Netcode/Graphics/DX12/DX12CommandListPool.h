#pragma once

#include "DX12Common.h"
#include <vector>
#include <string>

#define DEBUG_NAME(name, idx) (name + std::to_wstring(idx)).c_str()

namespace Netcode::Graphics::DX12 {

	struct CommandList {
		using CL_Type = ID3D12GraphicsCommandList3;
		using A_Type = ID3D12CommandAllocator;

		D3D12_COMMAND_LIST_TYPE type;
		com_ptr<CL_Type> commandList;
		com_ptr<A_Type> commandAllocator;

		CommandList(ID3D12Device * device, D3D12_COMMAND_LIST_TYPE type) : type{ type }, commandList{ nullptr }, commandAllocator{ nullptr } {
			DX_API("Failed to create command allocator")
				device->CreateCommandAllocator(type, IID_PPV_ARGS(commandAllocator.GetAddressOf()));

			DX_API("Failed to create command list")
				device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));
		}

		CommandList(const CommandList &) = default;
		CommandList & operator=(const CommandList &) = default;
		~CommandList() = default;
	};

	class CommandListPool {
		com_ptr<ID3D12Device> device;

		std::vector<CommandList> direct;
		std::vector<CommandList> copy;
		std::vector<CommandList> compute;

	public:
		void SetDevice(com_ptr<ID3D12Device> dev) {
			device = std::move(dev);
			direct.clear();
			copy.clear();
			compute.clear();

			constexpr uint32_t numCopy = 4;
			constexpr uint32_t numCompute = 4;
			constexpr uint32_t numDirect = 16;

			direct.reserve(numDirect);
			copy.reserve(numCopy);
			compute.reserve(numCompute);
			
			for(uint32_t i = 0; i < numDirect; ++i) {
				direct.emplace_back(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
				direct.back().commandList->SetName(DEBUG_NAME(L"Direct#", i));
			}

			for(uint32_t i = 0; i < numCopy; ++i) {
				copy.emplace_back(device.Get(), D3D12_COMMAND_LIST_TYPE_COPY);
				copy.back().commandList->SetName(DEBUG_NAME(L"Copy#", i));
			}

			for(uint32_t i = 0; i < numCompute; ++i) {
				compute.emplace_back(device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
				compute.back().commandList->SetName(DEBUG_NAME(L"Compute#", i));
			}
		}

		void Return(CommandList cl) {

			DX_API("Failed to reset command allocator")
				cl.commandAllocator->Reset();

			DX_API("Failed to reset command list")
				cl.commandList->Reset(cl.commandAllocator.Get(), nullptr);

			switch(cl.type) {
				case D3D12_COMMAND_LIST_TYPE_DIRECT:
				{
					direct.push_back(std::move(cl));
				break;
				}
				case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				{
					compute.push_back(std::move(cl));
				}
				break;
				case D3D12_COMMAND_LIST_TYPE_COPY:
				{
					copy.push_back(std::move(cl));
				}
				break;
				default:
					break;
			}
		}

		CommandList GetDirect() {
			if(direct.empty()) {
				direct.emplace_back(device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
			}

			CommandList cl = direct.back();

			direct.pop_back();

			return cl;
		}

		CommandList GetCompute() {
			if(compute.empty()) {
				compute.emplace_back(device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
			}

			CommandList cl = compute.back();

			compute.pop_back();

			return cl;
		}

		CommandList GetCopy() {
			if(copy.empty()) {
				copy.emplace_back(device.Get(), D3D12_COMMAND_LIST_TYPE_COPY);
			}

			CommandList cl = copy.back();

			copy.pop_back();

			return cl;
		}
	};

}
