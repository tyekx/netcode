#pragma once

#include <functional>
#include <dxgiformat.h>

enum class ResourceType : unsigned {
	TRANSIENT_DEFAULT,
	TRANSIENT_UPLOAD,
	TRANSIENT_READBACK,
	PERMANENT_DEFAULT,
	PERMANENT_UPLOAD,
	PERMANENT_READBACK
};

enum class ResourceFlags : unsigned {
	NONE = 0,
	ALLOW_RENDER_TARGET = 1,
	ALLOW_DEPTH_STENCIL = 2,
	ALLOW_UNORDERED_ACCESS = 4,
	DENY_SHADER_RESOURCE = 8
	/*
	Expand on demand, this should directly translate to D3D12_RESOURCE_FLAGS
	*/
};

#undef GENERIC_READ
enum class ResourceState : unsigned {
	COMMON = 0,
	VERTEX_AND_CONSTANT_BUFFER = 0x1,
	INDEX_BUFFER = 0x2,
	RENDER_TARGET = 0x4,
	UNORDERED_ACCESS = 0x8,
	DEPTH_WRITE = 0x10,
	DEPTH_READ = 0x20,
	NON_PIXEL_SHADER_RESOURCE = 0x40,
	PIXEL_SHADER_RESOURCE = 0x80,
	STREAM_OUT = 0x100,
	INDIRECT_ARGUMENT = 0x200,
	COPY_DEST = 0x400,
	COPY_SOURCE = 0x800,
	RESOLVE_DEST = 0x1000,
	RESOLVE_SOURCE = 0x2000,
	RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
	SHADING_RATE_SOURCE = 0x1000000,
	GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
	PRESENT = 0,
	PREDICATION = 0x200
};

class RenderContext {
public:
	void SetPipelineState(int handle) { }
	void BindCbuffer(int slot, int cbufferHandle) { }

	void SetPrimitiveTopology() { }
	void Render(int handle) { }
	void SetStreamOutput() { }
	void ClearStreamOutput() { }

	int CreateResource(std::size_t size, ResourceState initState) { }

	int CreateResource(std::size_t size, ResourceState initState, ResourceFlags flags) {

	}

	void ClearUAV(int handle) {

	}

};

class RenderGraphBuilder {
public:
	void ReadTexture(int textureHandle) { }
	void WriteTexture(int textureHandle) { }

	void ReadBuffer(int buffer) { }

	void CreateCbuffer(int handle) { }
	void CreateTexture(int textureHandle) { }

	int CreateResource(std::size_t size, ResourceState initState) {
		
	}

	int CreateResource(std::size_t size, ResourceState initState, ResourceFlags flags) {

	}

	void WriteBuffer(int bufferHandle) { }

	void DenyUserInput() {  }

	void SetPipelineState(int pipelineStateHandle) { }
};

struct RenderData {
	int NextUserInput() { }
	bool HasUserInput() { }
	void SetOutput(int handle) { }
};

using setup_func_t = std::function<void(RenderGraphBuilder &)>;
using render_func_t = std::function<void(RenderContext &, RenderData&)>;

class RenderPass {
public:
	setup_func_t setup;
	render_func_t render;

};

class PipelineBuilder {
public:
	RenderPass * CreateRenderPass();
	int CreatePipelineState();

	void SetBlendState();
	void SetRasterizerState();
	void SetVertexShader();
	void SetPixelShader();
};


