#pragma once

#include <DirectXMath.h>
#include "Common.h"

namespace Netcode::Graphics {

	class BasicGeometry {
		~BasicGeometry() = delete;
		BasicGeometry() = delete;

	public:
		/*
		Fills a vertices data buffer's position values. Creates a unit length line on the Z+ axis
		Designed to work with linelist primitive topology
		writes 2 vertices, 12 bytes (3 floats, x,y,z)
		*/
		static void CreateLine(_Out_writes_(2 * stride) void * dstData, unsigned int stride, unsigned int positionDataOffset = 0);


		/*
		Fills a vertices data buffer's position values. Creates a box around the origo with the given halfextents
		Designed to work with linelist primitive topology
		writes 24 vertices
		*/
		static void CreateBoxWireframe(_Out_writes_(24 * stride) void * dstData, unsigned int stride, const DirectX::XMFLOAT3 & halfExtents, unsigned int positionDataOffset = 0);

		/*
		Fills a vertices data buffer's position values. Creates a grid
		Designed to work with linelist primitive topology
		*/
		static void CreatePlaneWireframe(_Out_writes_(4 * (gridSectionCount + 1) * stride) void * dstData, unsigned int stride, unsigned int gridSectionCount, unsigned int positionDataOffset = 0);

		/*
		Fills a vertices data buffer's position values. Creates a capsule with the given arguments
		Designed to work with linelist primitive topology
		*/
		static void CreateCapsuleWireframe(_Out_writes_(108 * stride) void * dstData, unsigned int stride, const DirectX::XMFLOAT2 & args, unsigned int positionDataOffset = 0);

		static void CreateSphereWireFrame(void * dstData, uint32_t stride, float radius, uint32_t positionDataOffset = 0);

	};

}
