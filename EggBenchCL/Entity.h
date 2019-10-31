#pragma once

#include <Egg/Mesh/Shaded.h>
#include <Egg/ConstantBuffer.hpp>
#include <DirectXMath.h>
#include "Prototype.h"

class Entity {
public:
	CbufferAlloc<PerObjectCb> objCb;
	ReadOptimizedShadedMesh shaded;
	float animTime;
	DirectX::XMFLOAT3 direction;
	DirectX::XMFLOAT3 scale;
	//com_ptr<ID3D12DescriptorHeap> texHeap;

	~Entity() {
		ReleaseResources();
	}

	void Animate(float dt) {
		animTime = fmodf(animTime + dt, 4.0f);

		DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
		DirectX::XMVECTOR sc = DirectX::XMLoadFloat3(&scale);
		dir = DirectX::XMVector3Normalize(dir);
		dir = DirectX::XMVectorScale(dir, 200.0f * animTime);
		sc = DirectX::XMVectorScale(sc, 50.0f * ( (4.0f - animTime) / 4.0f));

		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(dir);
		DirectX::XMMATRIX scaling = DirectX::XMMatrixScalingFromVector(sc);

		scaling = DirectX::XMMatrixMultiply(scaling, translation);

		DirectX::XMStoreFloat4x4(&objCb->Model, DirectX::XMMatrixTranspose(scaling));
	}

	void ReleaseResources() {
	}

	void CreateResources(ID3D12Device * device) {
	}

	void Draw(ID3D12GraphicsCommandList * cl) {
		//shaded->GetMatPtr()->ApplyPipelineState(cl);
		cl->SetGraphicsRootConstantBufferView(0, objCb.addr);
		shaded.geometry->Draw(cl);
	}

};
