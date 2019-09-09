#pragma once

#include "Common.h"
#include "Mesh/Shaded.h"
#include "App.h"
#include "Shader.h"

namespace Egg {

	class SimpleApp : public App {
	protected:
		com_ptr<ID3D12CommandAllocator> commandAllocator;
		com_ptr<ID3D12GraphicsCommandList> commandList;
		com_ptr<ID3D12Resource> depthStencilBuffer;
		com_ptr<ID3D12DescriptorHeap> dsvHeap;

		std::unique_ptr<PsoManager> psoManager;

		virtual void PopulateCommandList() = 0;

		void WaitForPreviousFrame() {
			const UINT64 fv = fenceValue;
			DX_API("Failed to signal from command queue")
				commandQueue->Signal(fence.Get(), fv);

			fenceValue++;

			if(fence->GetCompletedValue() < fv) {
				DX_API("Failed to sign up for event completion")
					fence->SetEventOnCompletion(fv, fenceEvent);
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			frameIndex = swapChain->GetCurrentBackBufferIndex();
		}


	public:

		virtual void Render() override {
			PopulateCommandList();

			// Execute
			ID3D12CommandList * cLists[] = { commandList.Get() };
			commandQueue->ExecuteCommandLists(_countof(cLists), cLists);

			DX_API("Failed to present swap chain")
				swapChain->Present(1, 0);

			// Sync
			WaitForPreviousFrame();
		}

		virtual void CreateResources() override {
			App::CreateResources();

			psoManager.reset(new PsoManager{ device });

			DX_API("Failed to create command allocator")
				device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf()));

			DX_API("Failed to greate graphics command list")
				device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));

			commandList->Close();

			WaitForPreviousFrame();
		}

		virtual void ReleaseResources() override {
			psoManager.reset(nullptr);
			commandList.Reset();
			fence.Reset();
			commandAllocator.Reset();
			App::ReleaseResources();
		}

		virtual void CreateSwapChainResources() override {
			Egg::App::CreateSwapChainResources();

			D3D12_DESCRIPTOR_HEAP_DESC dsHeapDesc = {};
			dsHeapDesc.NumDescriptors = 1;
			dsHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			DX_API("Failed to create depth stencil descriptor heap")
				device->CreateDescriptorHeap(&dsHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf()));


			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
			depthOptimizedClearValue.DepthStencil.Stencil = 0;

			DX_API("Failed to create Depth Stencil buffer")
				device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, scissorRect.right, scissorRect.bottom, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					&depthOptimizedClearValue,
					IID_PPV_ARGS(depthStencilBuffer.GetAddressOf()));

			depthStencilBuffer->SetName(L"Depth Stencil Buffer");

			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

			device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}

		virtual void ReleaseSwapChainResources() override {
			depthStencilBuffer.Reset();
			dsvHeap.Reset();
			Egg::App::ReleaseSwapChainResources();
		}

		virtual void Resize(int width, int height) override {
			WaitForPreviousFrame();
			App::Resize(width, height);
		}


		virtual void Destroy() override {
			WaitForPreviousFrame();
			App::Destroy();
		}
	};

}
