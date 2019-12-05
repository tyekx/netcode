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

	class IResourceUploader : public IResource {
	public:
		virtual ~IResourceUploader() = default;
		virtual void Upload(const D3D12_RESOURCE_DESC & resourceDesc, ID3D12Resource * destResource, void * cpuResource, UINT sizeInBytes) = 0;
		virtual void Transition(ID3D12Resource * resource, D3D12_RESOURCE_STATES preState, D3D12_RESOURCE_STATES postState) = 0;
	};

	class IResourceUploadBatch : public IResourceUploader {
	public:
		virtual void Prepare() = 0;
		virtual void Process(ID3D12CommandQueue * directQueue, ID3D12CommandQueue * copyQueue) = 0;
	};

	/*
	Interface for upload resources
	these resources should be changed rarely changed, and will be placed in default heap
	*/
	class IUploadResource : public IResource {
	public:
		virtual ~IUploadResource() = default;
		virtual void UploadResources(IResourceUploader* copyCommandList) = 0;
		virtual void ReleaseUploadResources() = 0;
	};

	/*
	Interface for textures
	*/
	class ITexture : public IUploadResource {
	public:
		virtual ~ITexture() = default;
		virtual const D3D12_RESOURCE_DESC & GetDesc() const = 0;
		virtual D3D12_SHADER_RESOURCE_VIEW_DESC GetSRV() const = 0;
		virtual ID3D12Resource * GetResource() const = 0;
	};

	struct VertexLODLevel {
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		UINT verticesCount;
	};

	/*
	Abstract Vertex Buffer
	*/
	class AVBuffer : public IUploadResource {
	protected:
		VertexLODLevel lodLevels[8];
		UINT lodLevelsLength;
	public:
		virtual ~AVBuffer() = default;

		UINT GetLODCount() const {
			return lodLevelsLength;
		}

		const VertexLODLevel & GetLOD(UINT idx) const {
			return lodLevels[idx];
		}
	};

	struct IndexLODLevel {
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		UINT indexCount;
	};

	/*
	Abstract Index Buffer
	*/
	class AIBuffer : public IUploadResource {
	protected:
		IndexLODLevel lodLevels[8];
		UINT lodLevelsLength;
	public:
		virtual ~AIBuffer() = default;

		UINT GetLODCount() const {
			return lodLevelsLength;
		}

		const IndexLODLevel & GetLOD(UINT idx) const {
			return lodLevels[idx];
		}
	};


}
