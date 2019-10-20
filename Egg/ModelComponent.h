#pragma once

#include "Mesh/Shaded.h"
#include "ConstantBuffer.hpp"

#include <DirectXMath.h>


struct ModelComponent {
	Egg::Mesh::Shaded::P ShadedMesh;
	Egg::ConstantBuffer<PerObjectCb> Cbuffer;

	ModelComponent(ID3D12Device * device) : ShadedMesh{ nullptr }, Cbuffer{} {
		Cbuffer.CreateResources(device);
	}
};
