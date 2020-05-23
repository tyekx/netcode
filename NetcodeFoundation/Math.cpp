#include "Math.h"

namespace Netcode {

	Matrix NC_MATH_CALLCONV AffineTransformation(Vector3 scaling, Vector3 rotationOrigin, Quaternion rotation, Vector3 translation) {
		return DirectX::XMMatrixAffineTransformation(scaling.v, rotationOrigin.v, rotation.q, translation.v);
	}

	Matrix NC_MATH_CALLCONV AffineTransformation(Vector3 scaling, Quaternion rotation, Vector3 translation) {
		return DirectX::XMMatrixAffineTransformation(scaling.v, DirectX::g_XMZero, rotation.q, translation.v);
	}

	Matrix NC_MATH_CALLCONV TranslationMatrix(Vector3 translation)
	{
		return DirectX::XMMatrixTranslationFromVector(translation.v);
	}

	Matrix NC_MATH_CALLCONV LookToMatrix(Vector3 eyePos, Vector3 lookDir, Vector3 upVector) {
		return DirectX::XMMatrixLookToRH(eyePos.v, lookDir.v, upVector.v);
	}

	Matrix NC_MATH_CALLCONV PerspectiveFovMatrix(float horizontalFovInRadians, float aspect, float nearPlane, float farPlane)
	{
		return DirectX::XMMatrixPerspectiveFovRH(horizontalFovInRadians, aspect, nearPlane, farPlane);
	}

	Matrix NC_MATH_CALLCONV OrtographicMatrix(float viewWidth, float viewHeight, float nearPlane, float farPlane) {
		return DirectX::XMMatrixOrthographicRH(viewWidth, viewHeight, nearPlane, farPlane);
	}

	Quaternion NC_MATH_CALLCONV DecomposeRotation(Matrix mat)
	{
		return DirectX::XMQuaternionRotationMatrix(mat.mat);
	}

}
