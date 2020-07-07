#pragma once

#include <limits>
#include <DirectXCollision.h>
#include <algorithm>

/*
Generates a bounding box from a set of vertices
*/
class BoundingBoxGenerator {
	Netcode::Float3 minPoint;
	Netcode::Float3 maxPoint;

public:
	BoundingBoxGenerator() :
		minPoint{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() },
		maxPoint{ std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() } {

	}

	void UpdateForPoint(const Netcode::Float3 & p) {
		minPoint.x = std::min(minPoint.x, p.x);
		minPoint.y = std::min(minPoint.y, p.y);
		minPoint.z = std::min(minPoint.z, p.z);

		maxPoint.x = std::max(maxPoint.x, p.x);
		maxPoint.y = std::max(maxPoint.y, p.y);
		maxPoint.z = std::max(maxPoint.z, p.z);
	}

	DirectX::BoundingBox GetBoundingBox() const {
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

};
