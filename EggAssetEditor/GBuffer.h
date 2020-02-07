#pragma once

#include <cstdint>

struct GBuffer {
	uint64_t vertexBuffer;
	uint64_t indexBuffer;
	uint64_t vertexCount;
	uint64_t indexCount;
};
