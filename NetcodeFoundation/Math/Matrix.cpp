#include "Matrix.h"
#include "Vector4.h"

namespace Netcode {

	Matrix Matrix::Transpose() const {
		return Matrix{ DirectX::XMMatrixTranspose(mat) };
	}

	Matrix NC_MATH_CALLCONV Matrix::Invert() const {
		Netcode::Vector4 det = DirectX::XMMatrixDeterminant(mat);

		NotNullVector<Vector4> notNullCheck{ det };

		return DirectX::XMMatrixInverse(&det.v, mat);
	}

}
