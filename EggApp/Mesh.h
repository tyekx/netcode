#pragma once

#include <Egg/Common.h>
#include <Egg/GraphicsContexts.h>

#include <DirectXCollision.h>

struct GBuffer {
	uint64_t vertexBuffer;
	uint64_t indexBuffer;
	uint64_t vertexCount;
	uint64_t indexCount;
};

class Mesh {
public:

	DirectX::BoundingBox boundingBox;

	GBuffer lods[4];
	uint32_t selectedLod;
	uint32_t lodCount;

	uint32_t vertexType;
	uint32_t vertexSize;

	void AddLOD(const GBuffer & lod) {
		if(lodCount > 3) {
			return;
		}
		lods[lodCount] = lod;
		lodCount += 1;
	}

	const GBuffer & GetGBuffer() const {
		return lods[selectedLod];
	}

};
