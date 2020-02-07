#pragma once

#include <limits>
#include <DirectXCollision.h>
#include <algorithm>

/*
Generates a bounding box from a set of vertices
*/
class BoundingBoxGenerator {
	DirectX::XMFLOAT3 minPoint;
	DirectX::XMFLOAT3 maxPoint;

public:
	BoundingBoxGenerator() :
		minPoint{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() },
		maxPoint{ std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() } {

	}

	void UpdateForPoint(const DirectX::XMFLOAT3 & p) {
		minPoint.x = std::min(minPoint.x, p.x);
		minPoint.y = std::min(minPoint.y, p.y);
		minPoint.z = std::min(minPoint.z, p.z);

		maxPoint.x = std::max(maxPoint.x, p.x);
		maxPoint.y = std::max(maxPoint.y, p.y);
		maxPoint.z = std::max(maxPoint.z, p.z);
	}

	DirectX::BoundingBox GetBoundingBox() const {
		DirectX::XMFLOAT3 extents = DirectX::XMFLOAT3{
			(maxPoint.x - minPoint.x) / 2.0f,
			(maxPoint.y - minPoint.y) / 2.0f,
			(maxPoint.z - minPoint.z) / 2.0f
		};

		DirectX::XMFLOAT3 center = DirectX::XMFLOAT3{
			(maxPoint.x - extents.x),
			(maxPoint.y - extents.y),
			(maxPoint.z - extents.z)
		};

		return DirectX::BoundingBox{ center, extents };
	}

};
