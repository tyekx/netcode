#pragma once

#include <Netcode/Modules.h>
#include <map>
#include <string>
#include <Netcode/Vertex.h>
#include <Netcode/Utility.h>
#include "GameObject.h"
#include "Mesh.h"

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;

struct GpuAnimationKey {
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT4 scale;
};
