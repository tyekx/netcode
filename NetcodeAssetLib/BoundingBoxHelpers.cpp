#include "BoundingBoxHelpers.h"
#include <limits>

BoundingBoxGenerator::BoundingBoxGenerator() :
	minPoint{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() },
	maxPoint{ std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() } {

}

void BoundingBoxGenerator::UpdateForPoint(const Netcode::Float3 & p) {
	Netcode::Vector3 minVec = minPoint;
	Netcode::Vector3 maxVec = maxPoint;

	minPoint = minVec.Min(p);
	maxPoint = maxVec.Max(p);
}

DirectX::BoundingBox BoundingBoxGenerator::GetBoundingBox() const {
	Netcode::Float3 extents = Netcode::Float3{
		(maxPoint.x - minPoint.x) / 2.0f,
		(maxPoint.y - minPoint.y) / 2.0f,
		(maxPoint.z - minPoint.z) / 2.0f
	};

	Netcode::Float3 center = Netcode::Float3{
		(maxPoint.x - extents.x),
		(maxPoint.y - extents.y),
		(maxPoint.z - extents.z)
	};

	return DirectX::BoundingBox{ center, extents };
}

DirectX::BoundingBox MergeBoundingBoxes(const DirectX::BoundingBox & lhs, const DirectX::BoundingBox & rhs)
{
	Netcode::Vector3 c1 = DirectX::XMLoadFloat3(&lhs.Center);
	Netcode::Vector3 e1 = DirectX::XMLoadFloat3(&lhs.Extents);

	Netcode::Vector3 c2 = DirectX::XMLoadFloat3(&rhs.Center);
	Netcode::Vector3 e2 = DirectX::XMLoadFloat3(&rhs.Extents);

	Netcode::Vector3 minP1 = c1 - e1;
	Netcode::Vector3 maxP1 = c1 + e1;

	Netcode::Vector3 minP2 = c2 - e2;
	Netcode::Vector3 maxP2 = c2 + e2;

	BoundingBoxGenerator bg;
	bg.UpdateForPoint(minP1);
	bg.UpdateForPoint(maxP1);
	bg.UpdateForPoint(minP2);
	bg.UpdateForPoint(maxP2);

	return bg.GetBoundingBox();
}
