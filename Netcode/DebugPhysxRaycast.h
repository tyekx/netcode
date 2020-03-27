#pragma once

#include "DebugPhysxActor.h"
#include <DirectXMath.h>

namespace Netcode {

	__declspec(align(16)) struct PerRayCb {
		constexpr static int id = 14;

		DirectX::XMFLOAT3 direction;
		float length;
		DirectX::XMFLOAT3 startPos;
		float timeLeft;
		DirectX::XMFLOAT3 color;
	};

	/*

	class DebugPhysxRaycast {
		Egg::Material * debugMaterial;
		Egg::Mesh * rayGeometry;
		Egg::ConstantBuffer<PerRayCb> rayCb;
		float timeRemaining;
	public:

		DebugPhysxRaycast(ID3D12Device * device, Egg::Material * mat, Egg::Mesh * geom,
						  float lifeTime, const DirectX::XMFLOAT3 & dir, const DirectX::XMFLOAT3 & startPos, const DirectX::XMFLOAT3 & color, float length = 1000.0f
						  ) : debugMaterial{ mat }, rayGeometry{ geom }, rayCb{}, timeRemaining{ lifeTime } {
			rayCb.CreateResources(device);
			rayCb->direction = dir;
			rayCb->length = length;
			rayCb->startPos = startPos;
			rayCb->timeLeft = lifeTime;
			rayCb->color = color;
			rayCb.Upload();
		}

		~DebugPhysxRaycast() {
			rayCb.ReleaseResources();
		}

		DebugPhysxRaycast(const DebugPhysxRaycast &) = delete;

		void Update(float dt) {
			timeRemaining -= dt;
			if(IsAlive()) {
				rayCb->timeLeft = timeRemaining;
				rayCb.Upload();
			}
		}

		bool IsAlive() {
			return timeRemaining > 0.0f;
		}

		void ReleaseResources() {
			rayCb.ReleaseResources();
		}

		void Draw(ID3D12GraphicsCommandList * gcl) {
			debugMaterial->BindConstantBuffer(gcl, Egg::PerRayCb::id, rayCb.GetGPUVirtualAddress());

			rayGeometry->Draw(gcl);
		}
	};
	*/

}
