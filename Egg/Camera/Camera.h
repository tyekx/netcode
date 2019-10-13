#pragma once

#include "../Math/Math.h"

namespace Egg {
	namespace Camera {
		/// Basic camera interface, to be implemented by camera type classes.
		class BaseCamera {
			Egg::Math::Float4x4 ViewMatrix;
			Egg::Math::Float4x4 ProjMatrix;

			virtual void UpdateView() {
				ViewMatrix = Egg::Math::Float4x4::View(Position, Ahead, Egg::Math::Float3::UnitY);
			}

			virtual void UpdateProj() {
				ProjMatrix = Egg::Math::Float4x4::Proj(Fov, Aspect, NearPlane, FarPlane);
			}

		public:
			Egg::Math::Float3 Position;
			Egg::Math::Float3 Ahead;

			float Fov;
			float Aspect;
			float NearPlane;
			float FarPlane;

			virtual void UpdateMatrices() {
				UpdateProj();
				UpdateView();
			}

			void SetView(const Egg::Math::Float3 & position, const Egg::Math::Float3 & a) {
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

			virtual const Egg::Math::Float4x4& GetViewMatrix() {
				return ViewMatrix;
			}

			virtual const Egg::Math::Float4x4& GetProjMatrix() {
				return ProjMatrix;
			}

			virtual void SetAspect(float aspect) {
				Aspect = aspect;
				UpdateProj();
			}
		};
	}
}