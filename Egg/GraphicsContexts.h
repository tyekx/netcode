#pragma once

#include "ResourceEnums.h"
#include "ResourceDesc.h"
#include "HandleTypes.h"
#include "Common.h"
#include "UploadBatch.h"
#include <string>
#include <map>

namespace Egg::Graphics {

	enum class DisplayMode : unsigned {
		WINDOWED, FULLSCREEN, BORDERLESS
	};

	class IResourceContext {
	public:
		virtual ~IResourceContext() = default;

		virtual uint64_t CreateResource(const ResourceDesc & resource) = 0;
		virtual uint64_t CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
		virtual uint64_t CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
		virtual uint64_t CreateTexture2D(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) = 0;
		virtual uint64_t CreateTextureCube(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) = 0;

		virtual ResourceViewsRef CreateShaderResourceViews(uint32_t numDescriptors) = 0;
		virtual ResourceViewsRef CreateRenderTargetViews(uint32_t numDescriptors) = 0;
		virtual ResourceViewsRef CreateDepthStencilView() = 0;
		//virtual ResourceViewsRef CreateSamplers(uint32_t numDescriptors) = 0;

		virtual void SetDebugName(uint64_t resourceHandle, const wchar_t * name) = 0;

		/*
		Creates a Texture2D render target, every version invokes the one with the most arguments. Any argument not specified will fall back to its default:
		- width, height: backbuffer width, height
		- ResourceState: ResourceState::RENDER_TARGET
		- clearColor: (0,0,0,0)
		*/
		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) = 0;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType) = 0;

		/*
		Creates a Texture2D Depth stencil, every version invokes the one the most arguments. Any argument not specified will fall back to its default:
		- width, height: backbuffer width, height
		- ResourceState: ResourceState::DEPTH_WRITE
		- clearDepth: 1.0f
		- clearStencil: 0
		*/
		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) = 0;
		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) = 0;
		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) = 0;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) = 0;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) = 0;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType) = 0;

		virtual const ResourceDesc & QueryDesc(uint64_t handle) = 0;

		virtual void ReleaseResource(uint64_t handle) = 0;

		// for constant buffers that are frame-constants
		virtual uint64_t CreateConstantBuffer(size_t size) = 0;
		virtual void CopyConstants(uint64_t uploadResource, const void * srcData, size_t srcDataSizeInBytes) = 0;
		virtual void CopyConstants(uint64_t uploadResource, const void * srcData, size_t srcDataSizeInBytes, size_t dstOffsetInBytes) = 0;

		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState) = 0;
		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;

		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState) = 0;
		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
	};

	class IRenderContext {
	public:
		virtual ~IRenderContext() = default;

		virtual void SetRootSignature(RootSignatureRef rs) = 0;
		virtual void SetPipelineState(PipelineStateRef pso) = 0;
		virtual void SetVertexBuffer(uint64_t handle) = 0;
		virtual void SetIndexBuffer(uint64_t handle) = 0;
		virtual void DrawIndexed(uint32_t indexCount) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexOffset) = 0;
		virtual void Draw(uint32_t vertexCount) = 0;
		virtual void Draw(uint32_t vertexCount, uint32_t vertexOffset) = 0;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;

		virtual void ClearUnorderedAccessViewUint(uint64_t handle, const DirectX::XMUINT4 & values) = 0;
		virtual void ClearRenderTarget(uint8_t idx) = 0;
		virtual void ClearRenderTarget(uint8_t idx, const float * clearColor) = 0;
		virtual void ClearDepthOnly() = 0;
		virtual void ClearStencilOnly() = 0;
		virtual void ClearDepthStencil() = 0;

		virtual void SetStreamOutput(uint64_t handle) = 0;

		virtual void SetStreamOutputFilledSize(uint64_t handle, uint64_t byteOffset) = 0;
		virtual void ResetStreamOutput() = 0;

		virtual void SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil) = 0;
		
		virtual void SetStencilReference(uint8_t stencilValue) = 0;
		
		virtual void SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) = 0;
		// identical to SetViewport(0, width, 0, height);
		virtual void SetViewport(uint32_t width, uint32_t height) = 0;
		// resets to default viewport size = backbuffer viewport
		virtual void SetViewport() = 0;

		virtual void SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) = 0;
		virtual void SetScissorRect(uint32_t width, uint32_t height) = 0;
		virtual void SetScissorRect() = 0;

		// 0 means backbuffer RT/Depth
		virtual void SetRenderTargets(uint64_t renderTarget, uint64_t depthStencil) = 0;

		virtual void SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil) = 0;

		// for frame-constant constant buffers only that are allocated with CreateConstantBuffer
		virtual void SetConstantBuffer(int slot, uint64_t cbufferHandle) = 0;
		virtual void SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) = 0;
		virtual void SetShaderResources(int slot, std::initializer_list<uint64_t> shaderResourceHandles) = 0;
		virtual void SetShaderResources(int slot, ResourceViewsRef resourceView) = 0;

		virtual void ResourceBarrier(uint64_t handle, ResourceState before, ResourceState after) = 0;

		virtual void FlushResourceBarriers() = 0;
		virtual void BeginPass() = 0;
		virtual void EndPass() = 0;

		template<typename T>
		inline void SetConstants(int slot, const T & srcData) {
			SetConstants(slot, reinterpret_cast<const void *>(&srcData), sizeof(T));
		}

	};

	class IFrameContext {
	public:
		virtual ~IFrameContext() = default;

		virtual void SyncUpload(const UploadBatch & upload) = 0;

		virtual void Prepare() = 0;
		virtual void Render() = 0;
		virtual void Present() = 0;
	};

}
