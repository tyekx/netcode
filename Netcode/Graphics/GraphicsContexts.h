#pragma once

#include <NetcodeFoundation/Formats.h>
#include <NetcodeFoundation/Math.h>

#include <Netcode/HandleDecl.h>

namespace Netcode {
	class RenderPass;
	class FrameGraph;
}

namespace Netcode::Graphics {

	enum class DisplayMode : unsigned {
		WINDOWED, FULLSCREEN, BORDERLESS
	};

	class IResourceContext {
	public:
		virtual ~IResourceContext() = default;

		virtual void UseComputeContext() = 0;
		virtual void UseGraphicsContext() = 0;

		virtual void Writes(Ptr<GpuResource> resourceHandle) = 0;
		virtual void Reads(Ptr<GpuResource> resourceHandle) = 0;

		virtual void Writes(uintptr_t virtualDependency) = 0;
		virtual void Reads(uintptr_t virtualDependency) = 0;

		virtual void SetRenderPass(Ref<RenderPass> renderPass) = 0;
		virtual void ClearRenderPass() = 0;

		virtual Ref<UploadBatch> CreateUploadBatch() = 0;

		virtual Ref<GpuResource> CreateResource(const ResourceDesc & resource) = 0;
		virtual Ref<GpuResource> CreateReadbackBuffer(size_t size, ResourceType type, ResourceFlags flags) = 0;
		virtual Ref<GpuResource> CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
		virtual Ref<GpuResource> CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
		virtual Ref<GpuResource> CreateTexture2D(uint32_t width, uint32_t height, uint16_t mipLevels, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) = 0;
		virtual Ref<GpuResource> CreateTextureCube(uint32_t width, uint32_t height, uint16_t mipLevels, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) = 0;

		virtual Ref<GpuResource> CreateTexture2D(const Image * images) = 0;
		virtual Ref<GpuResource> CreateTexture2D(const Image * images, ResourceType resourceType) = 0;
		virtual Ref<GpuResource> CreateTexture2D(const Image * images, uint32_t mipLevels) = 0;
		virtual Ref<GpuResource> CreateTexture2D(const Image * images, uint32_t mipLevels, ResourceType resourceType) = 0;

		virtual Ref<ResourceViews> CreateShaderResourceViews(uint32_t numDescriptors) = 0;
		virtual Ref<ResourceViews> CreateRenderTargetViews(uint32_t numDescriptors) = 0;
		virtual Ref<ResourceViews> CreateDepthStencilView() = 0;
		//virtual ResourceViewsRef CreateSamplers(uint32_t numDescriptors) = 0;

		virtual void SetDebugName(Ref<GpuResource> resourceHandle, const wchar_t * name) = 0;

		/*
		Creates a Texture2D render target, every version invokes the one with the most arguments. Any argument not specified will fall back to its default:
		- width, height: backbuffer width, height
		- ResourceState: ResourceState::RENDER_TARGET
		- clearColor: (0,0,0,0)
		*/
		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) = 0;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) = 0;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType) = 0;

		/*
		Creates a Texture2D Depth stencil, every version invokes the one the most arguments. Any argument not specified will fall back to its default:
		- width, height: backbuffer width, height
		- ResourceState: ResourceState::DEPTH_WRITE
		- clearDepth: 1.0f
		- clearStencil: 0
		*/
		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) = 0;
		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) = 0;
		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) = 0;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) = 0;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) = 0;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) = 0;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType) = 0;

		// for constant buffers that are frame-constants
		virtual Ref<GpuResource> CreateConstantBuffer(size_t size) = 0;
		virtual void CopyConstants(Ref<GpuResource> uploadResource, const void * srcData, size_t srcDataSizeInBytes) = 0;
		virtual void CopyConstants(Ref<GpuResource> uploadResource, const void * srcData, size_t srcDataSizeInBytes, size_t dstOffsetInBytes) = 0;

		virtual void Readback(Ref<GpuResource> readbackResource, void * dstData, size_t dstDataSizeInBytes) = 0;
		virtual void Readback(Ref<GpuResource> readbackResource, void * dstData, size_t dstDataSizeInBytes, size_t srcOffsetInBytes) = 0;

		virtual Ref<GpuResource> CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState) = 0;
		virtual Ref<GpuResource> CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;

		virtual Ref<GpuResource> CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState) = 0;
		virtual Ref<GpuResource> CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
	};


	class IRenderContext {
	public:
		virtual ~IRenderContext() = default;

		virtual void SetRootSignature(Ref<RootSignature> rs) = 0;
		virtual void SetPipelineState(Ref<PipelineState> pso) = 0;
		virtual void SetVertexBuffer(Ref<GpuResource> handle) = 0;
		virtual void SetIndexBuffer(Ref<GpuResource> handle) = 0;
		virtual void DrawIndexed(uint32_t indexCount) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexOffset) = 0;
		virtual void Draw(uint32_t vertexCount) = 0;
		virtual void Draw(uint32_t vertexCount, uint32_t vertexOffset) = 0;
		virtual void Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ) = 0;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;

		virtual void ClearUnorderedAccessViewUint(Ref<GpuResource> handle, const UInt4 & values) = 0;
		virtual void ClearRenderTarget(uint8_t idx) = 0;
		virtual void ClearRenderTarget(uint8_t idx, const float * clearColor) = 0;
		virtual void ClearDepthOnly() = 0;
		virtual void ClearStencilOnly() = 0;
		virtual void ClearDepthStencil() = 0;

		virtual void SetStreamOutput(Ref<GpuResource> handle) = 0;

		virtual void SetStreamOutputFilledSize(Ref<GpuResource> handle, uint64_t byteOffset) = 0;
		virtual void ResetStreamOutput() = 0;

		virtual void SetRenderTargets(std::initializer_list<Ref<GpuResource>> handles, Ref<GpuResource> depthStencil) = 0;
		
		virtual void SetStencilReference(uint8_t stencilValue) = 0;
		
		virtual void SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) = 0;
		// identical to SetViewport(0, width, 0, height);
		virtual void SetViewport(uint32_t width, uint32_t height) = 0;
		// resets to default viewport size = backbuffer viewport
		virtual void SetViewport() = 0;

		virtual Float4 GetViewport() = 0;

		virtual void SetScissorRect(int32_t left, int32_t right, int32_t top, int32_t bottom) = 0;
		virtual void SetScissorRect(uint32_t width, uint32_t height) = 0;
		virtual void SetScissorRect() = 0;

		// 0 means backbuffer RT/Depth
		virtual void SetRenderTargets(std::nullptr_t rt, std::nullptr_t ds) = 0;

		virtual void SetRenderTargets(std::nullptr_t rt, Ref<ResourceViews> ds) = 0;
		virtual void SetRenderTargets(Ref<ResourceViews> rt, std::nullptr_t ds) = 0;
		virtual void SetRenderTargets(Ref<ResourceViews> renderTargets, Ref<ResourceViews> depthStencil) = 0;
		virtual void SetRenderTargets(std::nullptr_t rt, Ref<GpuResource> ds) = 0;
		virtual void SetRenderTargets(Ref<GpuResource> rt, std::nullptr_t ds) = 0;
		virtual void SetRenderTargets(Ref<GpuResource> renderTarget, Ref<GpuResource> depthStencil) = 0;

		// for frame-constant constant buffers only that are allocated with CreateConstantBuffer
		virtual void SetRootConstants(int slot, const void * srcData, uint32_t numConstants) = 0;
		virtual void SetConstantBuffer(int slot, Ref<GpuResource> cbufferHandle) = 0;
		virtual void SetConstants(int slot, uint64_t constantHandle) = 0;
		virtual uint64_t SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) = 0;
		virtual void SetShaderResources(int slot, std::initializer_list<Ref<GpuResource>> shaderResourceHandles) = 0;
		virtual void SetShaderResources(int slot, Ref<ResourceViews> resourceView) = 0;
		virtual void SetShaderResources(int slot, Ref<ResourceViews> resourceView, int descriptorOffset) = 0;
		virtual void CopyBufferRegion(Ref<GpuResource> dstResource, Ref<GpuResource> srcResource, size_t sizeInBytes) = 0;
		virtual void CopyBufferRegion(Ref<GpuResource> dstResource, size_t dstOffset, Ref<GpuResource> srcResource, size_t srcOffset, size_t sizeInBytes) = 0;

		virtual void UnorderedAccessBarrier(Ref<GpuResource> handle) = 0;
		virtual void ResourceBarrier(Ref<GpuResource> handle, ResourceState before, ResourceState after) = 0;

		virtual void FlushResourceBarriers() = 0;
		virtual void BeginPass() = 0;
		virtual void EndPass() = 0;

		template<typename T>
		inline uint64_t SetConstants(int slot, const T & srcData) {
			return SetConstants(slot, reinterpret_cast<const void *>(&srcData), sizeof(T));
		}
	};

	class IDebugContext {
	public:
		virtual ~IDebugContext() = default;

		virtual void DrawPoint(const Float3 & worldPos, float extents) = 0;
		virtual void DrawPoint(const Float3 & worldPos, float extents, bool depthEnabled) = 0;
		virtual void DrawPoint(const Float3 & worldPos, float extents, const Float4x4 & transform) = 0;
		virtual void DrawPoint(const Float3 & worldPos, float extents, const Float4x4 & transform, bool depthEnabled) = 0;

		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd) = 0;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, bool depthEnabled) = 0;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color) = 0;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, bool depthEnabled) = 0;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, const Float4x4 & transform) = 0;
		virtual void DrawLine(const Float3 & worldPosStart, const Float3 & worldPosEnd, const Float3 & color, const Float4x4 & transform,  bool depthEnabled) = 0;

		virtual void DrawSphere(Vector3 worldPosOrigin, float radius) = 0;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, bool depthEnabled) = 0;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color) = 0;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, bool depthEnabled) = 0;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, const Float4x4 & transform) = 0;
		virtual void DrawSphere(Vector3 worldPosOrigin, float radius, const Float3 & color, const Float4x4 & transform, bool depthEnabled) = 0;

		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents) = 0;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled) = 0;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color) = 0;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled) = 0;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform) = 0;
		virtual void DrawBoundingBox(Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform, bool depthEnabled) = 0;

		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents) = 0;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, bool depthEnabled) = 0;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color) = 0;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, bool depthEnabled) = 0;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform) = 0;
		virtual void DrawBox(Quaternion orientation, Vector3 worldPosOrigin, Vector3 halfExtents, const Float3 & color, const Float4x4 & transform, bool depthEnabled) = 0;

		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight) = 0;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, bool depthEnabled) = 0;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color) = 0;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, bool depthEnabled) = 0;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, const Float4x4 & transform) = 0;
		virtual void DrawCapsule(Quaternion rotation, Vector3 position, float radius, float halfHeight, const Float3 & color, const Float4x4 & transform, bool depthEnabled) = 0;

		virtual void UploadResources(IResourceContext * context) = 0;
		virtual void Draw(IRenderContext* context, const Float4x4 & viewProjMatrix) = 0;
	};

	class IFrameContext {
	public:
		virtual ~IFrameContext() = default;

		virtual void SyncUpload(Ref<UploadBatch> upload) = 0;

		virtual void Prepare() = 0;
		virtual void Run(Ref<FrameGraph> frameGraph, FrameGraphCullMode cullMode) = 0;
		virtual void DeviceSync() = 0;
		virtual void Present() = 0;
		virtual void CompleteFrame() = 0;
	};

}
