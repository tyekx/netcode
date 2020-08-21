#pragma once

#include <Netcode/HandleDecl.h>

struct GBuffer {
	Ref<Netcode::GpuResource> vertexBuffer;
	Ref<Netcode::GpuResource> indexBuffer;
	uint64_t vertexCount;
	uint64_t indexCount;
};
