#pragma once

#include "ResourceDesc.h"
#include "ResourceEnums.h"
#include "HandleTypes.h"
#include "Common.h"
#include <string>
#include <map>

namespace Egg::Graphics {

	class IGeometryContext {
	public:
		virtual ~IGeometryContext() = default;

		virtual HGEOMETRY CreateGeometry() = 0;
		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) = 0;
		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, DXGI_FORMAT format) = 0;
		virtual void AddInputElement(HGEOMETRY geometry, const char * name, DXGI_FORMAT format, unsigned int byteOffset) = 0;
		virtual void AddInputElement(HGEOMETRY geometry, const char * name, DXGI_FORMAT format) = 0;
	};

	class IShaderContext {
	public:
		virtual ~IShaderContext() = default;

		virtual HSHADER Load(const std::wstring & shaderPath) = 0;

		virtual HSHADER CreateVertexShader() = 0;
		virtual HSHADER CreatePixelShader() = 0;
		virtual HSHADER CreateGeometryShader() = 0;
		virtual HSHADER CreateDomainShader() = 0;
		virtual HSHADER CreateHullShader() = 0;

		virtual void SetEntrypoint(HSHADER shader, const std::string & entryFunction) = 0;
		virtual void SetSource(HSHADER shader, const std::wstring & shaderPath) = 0;
		virtual void SetDefinitions(HSHADER shader, const std::map<std::string, std::string> & defines) = 0;
	};

	class IPipelineContext {
	public:
		virtual ~IPipelineContext() = default;

		virtual HPSO CreatePipelineState() = 0;

		virtual void SetVertexShader(HPSO pso, HSHADER vertexShader) = 0;
		virtual void SetPixelShader(HPSO pso, HSHADER pixelShader) = 0;
		virtual void SetGeometryShader(HPSO pso, HSHADER geometryShader) = 0;
		virtual void SetHullShader(HPSO pso, HSHADER hullShader) = 0;
		virtual void SetDomainShader(HPSO pso, HSHADER domainShader) = 0;
		virtual void SetGeometry(HPSO pso, HGEOMETRY geometry) = 0;
	};

	class IResourceContext {
	public:
		virtual ~IResourceContext() = default;

		virtual uint64_t CreateResource(const ResourceDesc & resource) = 0;
		virtual uint64_t CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
		virtual uint64_t CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;

		virtual uint64_t CreateTexture2D() = 0;

		virtual uint64_t CreateRenderTarget() = 0;
		virtual uint64_t CreateDepthStencil() = 0;

		virtual const ResourceDesc & QueryDesc(uint64_t handle) = 0;

		virtual void CpuOnlyRenderPass() = 0;

		// for constant buffers that are frame-constants
		virtual uint64_t CreateConstantBuffer(size_t size) = 0;
		virtual void CopyConstants(uint64_t cbufferHandle, const void * srcData, size_t srcDataSizeInBytes) = 0;

		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type) = 0;
		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState) = 0;
		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;

		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type) = 0;
		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState) = 0;
		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) = 0;
	};

	class IRenderContext {
	public:
		virtual ~IRenderContext() = default;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;
		virtual void Draw(uint64_t handle) = 0;

		virtual void ClearTypedUAV(uint64_t handle) = 0;
		virtual void SetStreamOutput(uint64_t handle) = 0;

		virtual void SetStreamOutputFilledSize(uint64_t handle) = 0;
		virtual void ResetStreamOutput() = 0;

		// for frame-constant constant buffers only that are allocated with CreateConstantBuffer
		virtual void SetConstantBuffer(int slot, uint64_t cbufferHandle) = 0;
		virtual void SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) = 0;

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

		virtual void Prepare() = 0;
		virtual void Render() = 0;
		virtual void Present() = 0;
	};

}
