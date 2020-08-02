#include "Math.h"

namespace Netcode {

	float DegreesToRadians(float degrees)
	{
		constexpr float conversionConstant = Netcode::C_PI / 180.0f;
		return conversionConstant * degrees;
	}

	float RadiansToDegrees(float rads)
	{
		constexpr float conversionConstant = 180.0f / Netcode::C_PI;
		return conversionConstant * rads;
	}

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

	Matrix NC_MATH_CALLCONV ScaleMatrix(Vector3 scaling)
	{
		return DirectX::XMMatrixScalingFromVector(scaling.v);
	}

	Matrix NC_MATH_CALLCONV RotationMatrix(Quaternion rotation)
	{
		return DirectX::XMMatrixRotationQuaternion(rotation.q);
	}

	Matrix NC_MATH_CALLCONV LookToMatrix(Vector3 eyePos, Vector3 lookDir, Vector3 upVector) {
		return DirectX::XMMatrixLookToRH(eyePos.v, lookDir.v, upVector.v);
	}

	Matrix NC_MATH_CALLCONV LookAtMatrix(Vector3 eyePos, Vector3 lookPos, Vector3 upVector)
	{
		return DirectX::XMMatrixLookAtRH(eyePos.v, lookPos.v, upVector.v);
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
