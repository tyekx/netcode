#pragma once

#include <DirectXCollision.h>
#include <EggFoundation/Formats.h>

namespace Egg::Asset {

	struct InputElement {
		char semanticName[32];
		int32_t semanticIndex;
		DXGI_FORMAT format;
		uint32_t byteOffset;
	};

	struct LODLevel {
		uint32_t vertexBufferByteOffset;
		uint32_t vertexCount;
		uint32_t vertexBufferSizeInBytes;
		uint32_t indexBufferByteOffset;
		uint32_t indexCount;
		uint32_t indexBufferSizeInBytes;
	};

	struct Mesh {
		uint32_t materialId;
		uint32_t vertexSize;

		DirectX::BoundingBox boundingBox;

		uint32_t verticesSizeInBytes;
		uint32_t indicesSizeInBytes;
		uint32_t lodLevelsLength;
		uint32_t inputElementsLength;

		InputElement * inputElements;
		void * vertices;
		uint32_t * indices;
		LODLevel * lodLevels;
	};

}
