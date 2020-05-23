#pragma once 

#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"

namespace Netcode {

	Matrix NC_MATH_CALLCONV AffineTransformation(Vector3 scaling, Vector3 rotationOrigin, Quaternion rotation, Vector3 translation);

	Matrix NC_MATH_CALLCONV AffineTransformation(Vector3 scaling, Quaternion rotation, Vector3 translation);

	Matrix NC_MATH_CALLCONV TranslationMatrix(Vector3 translation);

	Matrix NC_MATH_CALLCONV LookToMatrix(Vector3 eyePos, Vector3 lookDir, Vector3 upVector);

	Matrix NC_MATH_CALLCONV PerspectiveFovMatrix(float horizontalFovInRadians, float aspect, float nearPlane, float farPlane);

	Matrix NC_MATH_CALLCONV OrtographicMatrix(float viewWidth, float viewHeight, float nearPlane, float farPlane);

	Quaternion NC_MATH_CALLCONV DecomposeRotation(Matrix mat);

}
