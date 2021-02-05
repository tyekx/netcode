#pragma once
#include "Common.h"

namespace Netcode {

	class Matrix {
	public:
		DirectX::XMMATRIX mat;

		~Matrix() noexcept = default;
		Matrix(const Matrix & rhs) noexcept = default;
		Matrix(Matrix &&) noexcept = default;
		Matrix & NC_MATH_CALLCONV operator=(const Matrix & rhs) noexcept = default;
		Matrix & NC_MATH_CALLCONV operator=(Matrix && rhs) noexcept = default;

		Matrix() noexcept : mat{ DirectX::XMMatrixIdentity() } { }
		Matrix(const Float4x4 & v) noexcept : mat{ DirectX::XMLoadFloat4x4(&v) } { }

		inline Matrix(const DirectX::XMMATRIX & m) noexcept : mat{ m } {}

		Matrix NC_MATH_CALLCONV Transpose() const;

		Matrix NC_MATH_CALLCONV Invert() const;

		Matrix NC_MATH_CALLCONV operator*(Matrix rhs) const noexcept {
			return DirectX::XMMatrixMultiply(mat, rhs.mat);
		}

		NC_MATH_CALLCONV operator Float4x4() const noexcept {
			DirectX::XMFLOAT4X4 a;
			DirectX::XMStoreFloat4x4(&a, mat);
			return a;
		}

		NC_MATH_CALLCONV operator DirectX::XMFLOAT4X4A() const noexcept {
			DirectX::XMFLOAT4X4A a;
			DirectX::XMStoreFloat4x4A(&a, mat);
			return a;
		}

		// rotation matrix along X axis
		static Matrix Pitch(float radians) noexcept {
			return DirectX::XMMatrixRotationX(radians);
		}

		// rotation matrix along Y axis
		static Matrix Yaw(float radians) noexcept {
			return DirectX::XMMatrixRotationY(radians);
		}

		// rotation matrix along Z axis
		static Matrix Roll(float radians) noexcept {
			return DirectX::XMMatrixRotationZ(radians);
		}
		
	};

}
