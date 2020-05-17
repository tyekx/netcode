#pragma once

#include <DirectXMath.h>

namespace Netcode {

	class GramSchmidt {
		DirectX::XMFLOAT3 b0;
		DirectX::XMFLOAT3 b1;
		DirectX::XMFLOAT3 b2;

		DirectX::XMVECTOR SubtractProjection(DirectX::XMVECTOR lhs, DirectX::XMVECTOR baseVec) {
			return DirectX::XMVectorSubtract(
				lhs,
				DirectX::XMVectorMultiply(
					DirectX::XMVectorMultiply(
						DirectX::XMVector3Dot(lhs, baseVec),
						DirectX::XMVectorReciprocal(DirectX::XMVector3Dot(baseVec, baseVec))
					),
					baseVec
				)
			);
		}

	public:

		const DirectX::XMFLOAT3 & GetBaseVec0() const {
			return b0;
		}

		const DirectX::XMFLOAT3 & GetBaseVec1() const {
			return b1;
		}

		const DirectX::XMFLOAT3 & GetBaseVec2() const {
			return b2;
		}

		GramSchmidt(const DirectX::XMFLOAT3 & e0, const DirectX::XMFLOAT3 & e1, const DirectX::XMFLOAT3 & e2) :
			b0{}, b1{}, b2{} {

			DirectX::XMVECTOR uVec = DirectX::XMLoadFloat3(&e0);

			DirectX::XMVECTOR vVec = DirectX::XMLoadFloat3(&e1);
			DirectX::XMVECTOR gsV = SubtractProjection(vVec, uVec);
			
 			DirectX::XMVECTOR wVec = DirectX::XMLoadFloat3(&e2);
			DirectX::XMVECTOR gsW = SubtractProjection(wVec, uVec);
			gsW = SubtractProjection(wVec, vVec);

			DirectX::XMStoreFloat3(&b0, DirectX::XMVector3Normalize(uVec));
			DirectX::XMStoreFloat3(&b1, DirectX::XMVector3Normalize(gsV));
			DirectX::XMStoreFloat3(&b2, DirectX::XMVector3Normalize(gsW));
		}

	};

}
