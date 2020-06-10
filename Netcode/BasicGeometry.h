#pragma once

#include <NetcodeFoundation/Math.h>
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
		static uint32_t CreateLine(void * dstData, uint32_t stride, uint32_t positionDataOffset = 0);


		/*
		Fills a vertices data buffer's position values. Creates a box around the origo with the given halfextents
		Designed to work with linelist primitive topology
		writes 24 vertices
		*/
		static uint32_t CreateBoxWireframe(void * dstData, uint32_t stride, const Netcode::Float3 & halfExtents, uint32_t positionDataOffset = 0);

		/*
		Fills a vertices data buffer's position values. Creates a grid
		Designed to work with linelist primitive topology
		*/
		static uint32_t CreatePlaneWireframe(void * dstData, uint32_t stride, uint32_t gridSectionCount, uint32_t positionDataOffset = 0);

		/*
		Fills a vertices data buffer's position values. Creates a capsule with the given arguments
		Designed to work with linelist primitive topology
		*/
		static uint32_t CreateCapsuleWireframe(void * dstData, uint32_t stride, const Netcode::Float2 & args, uint32_t positionDataOffset = 0);

		static uint32_t CreateGeoSphereWireframe(void * dstData, uint32_t stride, float radius, uint32_t positionDataOffset = 0);
		
		static uint32_t CreateSphereWireframe(void * dstData, uint32_t stride, float radius, uint32_t positionDataOffset = 0);

	};

}
