#pragma once

#include <DirectXCollision.h>
#include <NetcodeFoundation/Math.h>

/*
Generates a bounding box from a set of vertices
*/
class BoundingBoxGenerator {
	Netcode::Float3 minPoint;
	Netcode::Float3 maxPoint;

public:
	BoundingBoxGenerator();

	void UpdateForPoint(const Netcode::Float3 & p);

	DirectX::BoundingBox GetBoundingBox() const;
};

DirectX::BoundingBox MergeBoundingBoxes(const DirectX::BoundingBox & lhs, const DirectX::BoundingBox & rhs);
