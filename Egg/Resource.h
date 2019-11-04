#pragma once

#include "Common.h"

namespace Egg::Graphics::Resource {

	/*
	Interface for graphics resources
	these resources can vary in lifetime but should be able to free and reinitialize them in case of a device loss
	*/
	class IResource {
	public:
		virtual ~IResource() = default;
		virtual void CreateResources(ID3D12Device * device) = 0;
		virtual void ReleaseResources() = 0;
	};

	/*
	Interface for swapchain resources
	these resources need to be updated during a resize/maximize/minimize operation, or when a new swapchain is created
	*/
	class ISwapChainResource {
	public:
		virtual ~ISwapChainResource() = default;
		virtual void CreateSwapChainResources(ID3D12Device * device) = 0;
		virtual void ReleaseSwapChainResources() = 0;
		virtual void Resize(UINT width, UINT height) = 0;
	};


	/*
	Interface for upload resources
	these resources should be changed rarely changed, and will be placed in default heap
	*/
	class IUploadResource : public IResource {
	public:
		virtual ~IUploadResource() = default;
		virtual void UploadResources(ID3D12GraphicsCommandList * copyCommandList) = 0;
		virtual void ReleaseUploadResources() = 0;
	};

	/*
	Interface for textures
	*/
	class ITexture : public IUploadResource {
	public:
		virtual ~ITexture() = default;
		virtual const D3D12_RESOURCE_DESC & GetDesc() const = 0;
		virtual void CreateShaderResourceView(ID3D12Device * device, D3D12_CPU_DESCRIPTOR_HANDLE dHandle) = 0;
	};

	/*
	Abstract Vertex Buffer
	*/
	class AVBuffer : public IUploadResource {
	protected:
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	public:
		virtual ~AVBuffer() = default;

		const D3D12_VERTEX_BUFFER_VIEW & GetView() const {
			return vertexBufferView;
		}
	};

	/*
	Abstract Index Buffer
	*/
	class AIBuffer : public IUploadResource {
	protected:
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
	public:
		virtual ~AIBuffer() = default;

		const D3D12_INDEX_BUFFER_VIEW & GetView() const {
			return indexBufferView;
		}
	};


}
