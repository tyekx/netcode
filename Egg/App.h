#pragma once

#include "Common.h"
#include <vector>
#include <chrono>
#include "Stopwatch.h"
#include "Scene.h"

namespace Egg {

	enum class DisplayMode {
		WINDOWED, FULLSCREEN, BORDERLESS
	};

	class App {
	protected:
		com_ptr<ID3D12Device> device;
		com_ptr<IDXGISwapChain3> swapChain;
		com_ptr<ID3D12CommandQueue> commandQueue;

		// swap chain resources
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		D3D12_VIEWPORT viewPort;
		D3D12_RECT scissorRect;
		unsigned int backBufferDepth;
		unsigned int rtvDescriptorHandleIncrementSize;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		com_ptr<ID3D12DescriptorHeap> rtvDescriptorHeap;
		std::vector<com_ptr<ID3D12Resource>> renderTargets;
		float aspectRatio;

		// Sync objects
		com_ptr<ID3D12Fence> fence;
		HANDLE fenceEvent;
		unsigned long long fenceValue;
		unsigned int frameIndex;

		// timer objects
		Egg::Stopwatch stopwatch;
		std::unique_ptr<Egg::Scene> scene;

		float elapsedTime;
		DisplayMode mode;
		DirectX::XMUINT2 lastWindowedSize;

		/*
		* Internal resize implementation
		*/
		void ImplDetailResize(int w, int h, DisplayMode modeToSet, IDXGIOutput * optMonitor = nullptr) {
			ReleaseSwapChainResources();

			DX_API("Failed to get swap chain desc")
				swapChain->GetDesc(&swapChainDesc);

			BOOL fullscreenMode = FALSE;
			DX_API("Failed to get fullscreen state")
				swapChain->GetFullscreenState(&fullscreenMode, nullptr);

			if(!fullscreenMode) {
				lastWindowedSize.x = swapChainDesc.BufferDesc.Width;
				lastWindowedSize.y = swapChainDesc.BufferDesc.Height;
			}

			if(swapChainDesc.BufferDesc.Width != UINT(w) || swapChainDesc.BufferDesc.Height != UINT(h)) {
				DX_API("Failed to resize buffers")
					swapChain->ResizeBuffers(swapChainDesc.BufferCount, UINT(w), UINT(h), DXGI_FORMAT_UNKNOWN, swapChainDesc.Flags);
			}

			CreateSwapChainResources();
		}

	public:
		virtual ~App() = default;
		App() : device{ nullptr }, swapChain{ nullptr }, commandQueue{ nullptr }, swapChainDesc{}, viewPort{}, scissorRect{}, backBufferDepth{ }, rtvDescriptorHandleIncrementSize{}, rtvHandle{}, rtvDescriptorHeap{ nullptr },
			renderTargets{}, aspectRatio{ 1.0 }, fence{ nullptr }, fenceEvent{ NULL }, fenceValue{ 0 }, frameIndex{ 0 }, stopwatch{}, scene{ nullptr }, elapsedTime{ 0.0f }, mode{ DisplayMode::WINDOWED }, lastWindowedSize{ }{
		
			stopwatch.Reset();
		}

		void Run() {
			float deltaTime = stopwatch.Restart();
			elapsedTime += deltaTime;
			Update(deltaTime, elapsedTime);
			Render();
		}

		void SetScene(std::unique_ptr<Egg::Scene> sc) {
			scene = std::move(sc);
		}

		Egg::Scene * GetScene() const {
			return scene.get();
		}

		com_ptr<IDXGIOutput> GetContainingOutput() {
			com_ptr<IDXGIOutput> output;

			DX_API("Failed to get swap chain containing output")
				swapChain->GetContainingOutput(output.GetAddressOf());

			return output;
		}

		virtual void SetDisplayMode(DisplayMode newMode) {
			if(mode == newMode) {
				return;
			}

			com_ptr<IDXGIOutput> output;

			DX_API("Failed to get swap chain containing output")
				swapChain->GetContainingOutput(output.GetAddressOf());

			DXGI_OUTPUT_DESC outputDesc;

			DX_API("Failed to get output desc")
				output->GetDesc(&outputDesc);



			if((newMode == DisplayMode::FULLSCREEN)) {
				DX_API("Failed to set fullscreen mode")
					swapChain->SetFullscreenState(TRUE, output.Get());
			}

			if(newMode != DisplayMode::WINDOWED) {
				ImplDetailResize(outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left, outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top, newMode, output.Get());
			} else {
				output.Reset();
				ImplDetailResize(lastWindowedSize.x, lastWindowedSize.y, newMode, nullptr);
			}
			
			mode = newMode;
		}

		virtual void CreateSwapChainResources() {
			swapChain->GetDesc(&swapChainDesc);

			backBufferDepth = swapChainDesc.BufferCount;

			viewPort.TopLeftX = 0;
			viewPort.TopLeftY = 0;
			viewPort.Width = (float)swapChainDesc.BufferDesc.Width;
			viewPort.Height = (float)swapChainDesc.BufferDesc.Height;
			viewPort.MinDepth = 0.0f;
			viewPort.MaxDepth = 1.0f;

			lastWindowedSize.x = swapChainDesc.BufferDesc.Width;
			lastWindowedSize.y = swapChainDesc.BufferDesc.Height;

			aspectRatio = viewPort.Width / (float)viewPort.Height;

			scissorRect.left = 0;
			scissorRect.top = 0;
			scissorRect.right = swapChainDesc.BufferDesc.Width;
			scissorRect.bottom = swapChainDesc.BufferDesc.Height;

			// Create Render Target View Descriptor Heap, like a RenderTargetView** on the GPU. A set of pointers.

			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.NumDescriptors = backBufferDepth;
			rtvHeapDesc.NodeMask = 0;

			DX_API("Failed to create render target view descriptor heap")
				device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvDescriptorHeap.GetAddressOf()));

			rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			rtvDescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			// Create Render Target Views

			renderTargets.resize(backBufferDepth);

			for(unsigned int i = 0; i < backBufferDepth; ++i) {
				DX_API("Failed to get swap chain buffer")
					swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargets[i].GetAddressOf()));

				CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle{ rtvHandle };
				cpuHandle.Offset(i * rtvDescriptorHandleIncrementSize);

				device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, cpuHandle);
			}

			frameIndex = swapChain->GetCurrentBackBufferIndex();
		}

		virtual void ReleaseSwapChainResources() {
			for(com_ptr<ID3D12Resource> & i : renderTargets) {
				i.Reset();
			}
			renderTargets.clear();
			rtvDescriptorHeap.Reset();
		}

		virtual void Render() = 0;
		virtual void Update(float dt, float T) = 0;

		virtual void LoadAssets() { }

		virtual void ReleaseAssets() { }

		virtual void CreateResources() {
			DX_API("Failed to create fence")
				device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
			fenceValue = 1;

			fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(fenceEvent == NULL) {
				DX_API("Failed to create windows event") HRESULT_FROM_WIN32(GetLastError());
			}
		}

		virtual void ReleaseResources() {
			commandQueue.Reset();
			swapChain.Reset();
			device.Reset();
		}



		virtual void Resize(int width, int height) {
			ImplDetailResize(width, height, mode, nullptr);
		}

		virtual void Destroy() {
			ReleaseSwapChainResources();
			swapChain->SetFullscreenState(FALSE, nullptr);
			ReleaseResources();
			ReleaseAssets();
		}

		void SetCommandQueue(com_ptr<ID3D12CommandQueue> cQueue) {
			commandQueue = cQueue;
		}

		void SetDevice(com_ptr<ID3D12Device> dev) {
			device = dev;
		}

		void SetSwapChain(com_ptr<IDXGISwapChain3> sChain) {
			swapChain = sChain;
		}

		
		/*
		User input
		*/

		virtual void KeyPressed(uint32_t keyCode) { }
		virtual void KeyReleased(uint32_t keyCode) { }

		virtual void Focused() { }
		virtual void Blur() { }

		virtual void MouseMove(int x, int y) { }

		/*
		Editor
		*/
		virtual void SetSelection(int index) { }
	};

}
