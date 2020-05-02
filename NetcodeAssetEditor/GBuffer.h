#pragma once

#include <Netcode/HandleTypes.h>
#include <cstdint>

struct GBuffer {
	Netcode::GpuResourceRef vertexBuffer;
	Netcode::GpuResourceRef indexBuffer;
	uint64_t vertexCount;
	uint64_t indexCount;
};
