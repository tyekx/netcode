#pragma once


#include <DirectXMath.h>

namespace Egg {
	namespace Camera {
		/// Basic camera interface, to be implemented by camera type classes.
		class BaseCamera {
			DirectX::XMFLOAT4X4A ViewMatrix;
			DirectX::XMFLOAT4X4A ProjMatrix;

			virtual void UpdateView() {
				DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&Position);
				DirectX::XMVECTOR ahead = DirectX::XMLoadFloat3(&Ahead);
				DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&Up);
				DirectX::XMStoreFloat4x4(&ViewMatrix, DirectX::XMMatrixLookToRH(pos, ahead, up));
			}

			virtual void UpdateProj() {
				DirectX::XMStoreFloat4x4(&ProjMatrix, DirectX::XMMatrixPerspectiveFovRH(Fov, Aspect, NearPlane, FarPlane));
			}

		public:
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Ahead;
			DirectX::XMFLOAT3 Up;

			float Fov;
			float Aspect;
			float NearPlane;
			float FarPlane;

			BaseCamera() : ViewMatrix{}, ProjMatrix{}, Position{}, Ahead{}, Up{ 0.0f, 1.0f, 0.0f }, Fov{ 1.0f }, Aspect{ 1.0f }, NearPlane{ 0.0f }, FarPlane{ 1.0f } { }

			virtual void UpdateMatrices() {
				UpdateProj();
				UpdateView();
			}

			void SetView(const DirectX::XMFLOAT3 & position, const DirectX::XMFLOAT3 & a) {
				Position = position;
				Ahead = a;
				UpdateView();
			}

			void SetProj(float fov, float aspect, float nearPlane, float farPlane) {
				Fov = fov;
				Aspect = aspect;
				NearPlane = nearPlane;
				FarPlane = farPlane;
				UpdateProj();
			}

			virtual const DirectX::XMFLOAT4X4A & GetViewMatrix() {
				return ViewMatrix;
			}

			virtual const DirectX::XMFLOAT4X4A & GetProjMatrix() {
				return ProjMatrix;
			}

			virtual void SetAspect(float aspect) {
				Aspect = aspect;
				UpdateProj();
			}
		};
	}
}