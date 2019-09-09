#pragma once

#include "Mesh/Shaded.h"
#include "ConstantBuffer.hpp"
#include "Math/Float4x4.h"

__declspec(align(16)) struct PerObjectCb {
	Egg::Math::Float4x4 Model;
	Egg::Math::Float4x4 InvModel;
};

struct MeshComponent {
	Egg::Mesh::Shaded::P ShadedMesh;
	Egg::ConstantBuffer<PerObjectCb> Cbuffer;

	MeshComponent(ID3D12Device * device) : ShadedMesh{ nullptr }, Cbuffer{} {
		Cbuffer.CreateResources(device);
	}
};
