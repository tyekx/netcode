#pragma once

#include "GameObject.h"
#include <array>

class Scene {
public:
	std::array<GameObject, 1024> objects;
	std::size_t count;
	GameObject * camera;
	
	GameObject * Insert() {
		if(count == objects.max_size()) {
			return nullptr;
		}
		GameObject * obj = objects.data() + count;
		count += 1;
		return obj;
	}



	DirectX::XMMATRIX GetView(Transform * transform, Camera * camera) {
		DirectX::XMVECTOR eyePos = DirectX::XMLoadFloat3(&transform->position);
		DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
		DirectX::XMVECTOR ahead = DirectX::XMLoadFloat3(&camera->ahead);
		return DirectX::XMMatrixLookToRH(eyePos, ahead, up);
	}

	DirectX::XMMATRIX GetProj(Camera * camera) {
		return DirectX::XMMatrixPerspectiveFovRH(camera->fov, camera->aspect, camera->nearPlane, camera->farPlane);
	}

	void UpdatePerFrameCb(PerFrameCb * cb) {
		Transform * transform = camera->GetComponent<Transform>();
		Camera * camComponent = camera->GetComponent<Camera>();

		DirectX::XMMATRIX view = GetView(transform, camComponent);
		DirectX::XMMATRIX proj = GetProj(camComponent);


		DirectX::XMStoreFloat4x4A(&cb->View, DirectX::XMMatrixTranspose(view));
		DirectX::XMStoreFloat4x4A(&cb->Proj, DirectX::XMMatrixTranspose(proj));

		DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);
		DirectX::XMVECTOR vpDet = DirectX::XMMatrixDeterminant(vp);
		DirectX::XMMATRIX invVp = DirectX::XMMatrixInverse(&vpDet, vp);

		DirectX::XMStoreFloat4x4A(&cb->ViewProj, DirectX::XMMatrixTranspose(vp));
		DirectX::XMStoreFloat4x4A(&cb->ViewProjInv, DirectX::XMMatrixTranspose(invVp));
	}

	void SetCamera(GameObject * camObject) {
		camera = camObject;
	}
};
