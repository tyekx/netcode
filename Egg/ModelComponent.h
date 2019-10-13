#pragma once

#include "Mesh/Shaded.h"
#include "ConstantBuffer.hpp"
#include "Math/Float4x4.h"


__declspec(align(16)) struct PerFrameCb {
	constexpr static int id = 0;

	Egg::Math::Float4x4 View;
	Egg::Math::Float4x4 Proj;

};

__declspec(align(16)) struct PerObjectCb {
	constexpr static int id = 1;

	Egg::Math::Float4x4 Model;
	Egg::Math::Float4x4 InvModel;
};

struct ModelComponent {
	Egg::Mesh::Shaded::P ShadedMesh;
	Egg::ConstantBuffer<PerObjectCb> Cbuffer;

	ModelComponent(ID3D12Device * device) : ShadedMesh{ nullptr }, Cbuffer{} {
		Cbuffer.CreateResources(device);
	}
};
