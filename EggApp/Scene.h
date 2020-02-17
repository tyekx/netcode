#pragma once

#include "GameObject.h"
#include <array>

class Scene {
public:
	std::array<GameObject, 1024> objects;
	std::size_t count;
	GameObject * camera;

	PerFrameData perFrameData;
	SsaoData ssaoData;

	void Setup() {
		ssaoData.Offsets[0] = DirectX::XMFLOAT4A(+1.0f, +1.0f, +1.0f, 0.0f);
		ssaoData.Offsets[1] = DirectX::XMFLOAT4A(-1.0f, -1.0f, -1.0f, 0.0f);

		ssaoData.Offsets[2] = DirectX::XMFLOAT4A(-1.0f, +1.0f, +1.0f, 0.0f);
		ssaoData.Offsets[3] = DirectX::XMFLOAT4A(+1.0f, -1.0f, -1.0f, 0.0f);

		ssaoData.Offsets[4] = DirectX::XMFLOAT4A(+1.0f, +1.0f, -1.0f, 0.0f);
		ssaoData.Offsets[5] = DirectX::XMFLOAT4A(-1.0f, -1.0f, +1.0f, 0.0f);

		ssaoData.Offsets[6] = DirectX::XMFLOAT4A(-1.0f, +1.0f, -1.0f, 0.0f);
		ssaoData.Offsets[7] = DirectX::XMFLOAT4A(+1.0f, -1.0f, +1.0f, 0.0f);

		ssaoData.Offsets[8] = DirectX::XMFLOAT4A(-1.0f, 0.0f, 0.0f, 0.0f);
		ssaoData.Offsets[9] = DirectX::XMFLOAT4A(+1.0f, 0.0f, 0.0f, 0.0f);

		ssaoData.Offsets[10] = DirectX::XMFLOAT4A(0.0f, -1.0f, 0.0f, 0.0f);
		ssaoData.Offsets[11] = DirectX::XMFLOAT4A(0.0f, +1.0f, 0.0f, 0.0f);

		ssaoData.Offsets[12] = DirectX::XMFLOAT4A(0.0f, 0.0f, -1.0f, 0.0f);
		ssaoData.Offsets[13] = DirectX::XMFLOAT4A(0.0f, 0.0f, +1.0f, 0.0f);

		for(int i = 0; i < SsaoData::SAMPLE_COUNT; ++i)
		{
			float s = RandomFloat(0.3f, 1.0f);
			s = s * s;

			DirectX::XMVECTOR v = DirectX::XMVectorScale(DirectX::XMVector4Normalize(DirectX::XMLoadFloat4A(&ssaoData.Offsets[i])), s);

			DirectX::XMStoreFloat4A(&(ssaoData.Offsets[i]), v);
		}

		ssaoData.occlusionRadius = 0.5f;
		ssaoData.occlusionFadeStart = 0.2f;
		ssaoData.occlusionFadeEnd = 1.0f;
		ssaoData.surfaceEpsilon = 0.05f;
	}
	
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
		eyePos.m128_f32[1] += 180.0f;
		DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&camera->up);
		DirectX::XMVECTOR ahead = DirectX::XMLoadFloat3(&camera->ahead);
		return DirectX::XMMatrixLookToRH(eyePos, ahead, up);
	}

	DirectX::XMMATRIX GetProj(Camera * c) {
		return DirectX::XMMatrixPerspectiveFovRH(c->fov, c->aspect, c->nearPlane, c->farPlane);
	}

	void UpdatePerFrameCb() {
		Transform * transform = camera->GetComponent<Transform>();
		Camera * camComponent = camera->GetComponent<Camera>();

		const DirectX::XMMATRIX view = GetView(transform, camComponent);
		const DirectX::XMMATRIX proj = GetProj(camComponent);


		const DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);
		DirectX::XMVECTOR vpDet = DirectX::XMMatrixDeterminant(vp);
		const DirectX::XMMATRIX invVp = DirectX::XMMatrixInverse(&vpDet, vp);

		const DirectX::XMMATRIX tex{  0.5f,  0.0f, 0.0f, 0.0f,
									  0.0f, -0.5f, 0.0f, 0.0f,
									  0.0f,  0.0f, 1.0f, 0.0f,
									  0.5f,  0.5f, 0.0f, 1.0f };

		DirectX::XMVECTOR lookToV = DirectX::XMLoadFloat3(&camComponent->ahead);
		DirectX::XMVECTOR upV = DirectX::XMLoadFloat3(&camComponent->up);

		DirectX::XMMATRIX viewFromOrigo = DirectX::XMMatrixLookToRH(DirectX::g_XMZero, lookToV, upV);
		DirectX::XMMATRIX rayDir = DirectX::XMMatrixMultiply(viewFromOrigo, proj);

		DirectX::XMVECTOR rayDirDet = DirectX::XMMatrixDeterminant(rayDir);
		rayDir = DirectX::XMMatrixInverse(&rayDirDet, rayDir);

		DirectX::XMStoreFloat4x4A(&perFrameData.RayDir, DirectX::XMMatrixTranspose(rayDir));

		perFrameData.farZ = camComponent->farPlane;
		perFrameData.nearZ = camComponent->nearPlane;
		perFrameData.fov = camComponent->fov;
		perFrameData.aspectRatio = camComponent->aspect;

		DirectX::XMStoreFloat4x4A(&perFrameData.View, DirectX::XMMatrixTranspose(view));
		DirectX::XMStoreFloat4x4A(&perFrameData.Proj, DirectX::XMMatrixTranspose(proj));

		DirectX::XMStoreFloat4x4A(&perFrameData.ViewProj, DirectX::XMMatrixTranspose(vp));
		DirectX::XMStoreFloat4x4A(&perFrameData.ViewProjInv, DirectX::XMMatrixTranspose(invVp));

		const DirectX::XMFLOAT4 eyePos{ transform->position.x, transform->position.y, transform->position.z, 1.0f };

		DirectX::XMStoreFloat4A(&perFrameData.eyePos, DirectX::XMLoadFloat4(&eyePos));


		DirectX::XMStoreFloat4x4A(&perFrameData.ViewInv, DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply(proj, invVp) ));
		DirectX::XMStoreFloat4x4A(&perFrameData.ProjInv, DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply(invVp, view) ));

		//DirectX::XMVECTOR pDet = DirectX::XMMatrixDeterminant(proj);
		//const auto invP = DirectX::XMMatrixInverse(&pDet, proj);
		//DirectX::XMStoreFloat4x4A(&perFrameData.ProjInv, DirectX::XMMatrixTranspose(invP));

		DirectX::XMStoreFloat4x4A(&perFrameData.ProjTex, DirectX::XMMatrixTranspose( DirectX::XMMatrixMultiply( proj, tex ) ));
	}

	void SetCamera(GameObject * camObject) {
		camera = camObject;
	}
};
