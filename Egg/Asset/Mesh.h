#pragma once

#include <DirectXCollision.h>

namespace Egg::Asset {

	struct LODLevel {
		// not byte offset (need to multiply this by vertexSize to get byte offset)
		unsigned int vertexOffset;
		unsigned int vertexCount;
		unsigned int verticesLength;
		// not byte offset (need to multiply this by vertexSize to get byte offset)
		unsigned int indexOffset;
		unsigned int indexCount;
		unsigned int indicesLength;
	};

	struct Mesh {
		unsigned int materialId;
		unsigned int vertexType;
		unsigned int vertexSize;

		void * vertices;
		unsigned int * indices;

		unsigned int verticesLength;
		unsigned int indicesLength;

		DirectX::BoundingBox boundingBox;

		LODLevel * lodLevels;
		unsigned int lodLevelsLength;
	};

}
