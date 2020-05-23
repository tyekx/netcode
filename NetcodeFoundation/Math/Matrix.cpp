#include "Matrix.h"
#include "Vector4.h"

namespace Netcode {

	Matrix Matrix::Transpose() const {
		return Matrix{ DirectX::XMMatrixTranspose(mat) };
	}

	Matrix NC_MATH_CALLCONV Matrix::Invert() const {
		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(mat);

		NotNullVector<Vector4> notNullCheck{ Vector4{ det }	};

		return DirectX::XMMatrixInverse(&det, mat);
	}

}
