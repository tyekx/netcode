#pragma once

#include <Netcode/Common.h>
#include <Netcode/HandleTypes.h>

#include <DirectXCollision.h>

struct GBuffer {
	Ref<Netcode::GpuResource> vertexBuffer;
	Ref<Netcode::GpuResource> indexBuffer;
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
