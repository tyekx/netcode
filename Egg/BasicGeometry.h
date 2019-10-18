#pragma once

#include "Mesh/Geometry.h"

namespace Egg {

	struct WireframeVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
	};

	class BasicGeometry {
		~BasicGeometry() = delete;
		BasicGeometry() = delete;

	public:

		/*
		* This function creates a UnitZ for raycast debugging purposed, meat to be used in conjunction with DebugPhysxRayVS.
		*/
		static Egg::Mesh::Geometry::P CreateLine(ID3D12Device * device, const DirectX::XMFLOAT3 & color);

		/*
		* Creates a unit box wireframe around the origo
		*/
		static Egg::Mesh::Geometry::P CreateBoxWireframe(ID3D12Device * device, const DirectX::XMFLOAT3 & he, const DirectX::XMFLOAT3 & color);

		/*
		* Creates a unit plane wireframe on the xz plane from the origin
		*/
		static Egg::Mesh::Geometry::P CreatePlaneWireframe(ID3D12Device * device, unsigned int gridSectionCount, const DirectX::XMFLOAT3 & color);

		/*
		* Creates a capsule silhouette suitable for physics debugging and bounding box drawing as its a linelist has only position and color attributes,
		* use it with DebugPhysicsVS/PS
		*/
		static Egg::Mesh::Geometry::P CreateCapsuleWireframe(ID3D12Device * device, float height, float radius, const DirectX::XMFLOAT3 & color);

		/*
		* Create a Unit sized box instance, origin is (0,0,0),
		* each vertex has position, normal, texture
		*/
		static Egg::Mesh::Geometry::P CreateBox(ID3D12Device * device);


	};

}
