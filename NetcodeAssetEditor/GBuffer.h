#pragma once

#include <Netcode/Modules.h>
#include <Netcode/Graphics/GraphicsContexts.h>
#include <Netcode/Graphics/Material.h>
#include <Netcode/HandleTypes.h>
#include <memory>
#include <Netcode/URI/Texture.h>
#include "ConstantBufferTypes.h"

struct GBuffer {
	Ref<Netcode::GpuResource> vertexBuffer;
	Ref<Netcode::GpuResource> indexBuffer;
	uint64_t vertexCount;
	uint64_t indexCount;
};
