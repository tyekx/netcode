#pragma once

#include "Mesh/Shaded.h"
#include "ConstantBuffer.hpp"

#include <DirectXMath.h>


__declspec(align(16)) struct PerFrameCb {
	constexpr static int id = 0;

	DirectX::XMFLOAT4X4A ViewProj;

};

__declspec(align(16)) struct PerObjectCb {
	constexpr static int id = 1;

	DirectX::XMFLOAT4X4A Model;
	DirectX::XMFLOAT4X4A InvModel;
};

struct ModelComponent {
	Egg::Mesh::Shaded::P ShadedMesh;
	Egg::ConstantBuffer<PerObjectCb> Cbuffer;

	ModelComponent(ID3D12Device * device) : ShadedMesh{ nullptr }, Cbuffer{} {
		Cbuffer.CreateResources(device);
	}
};
