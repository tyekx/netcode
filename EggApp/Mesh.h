#pragma once

#include <Egg/Common.h>
#include <Egg/GraphicsContexts.h>

#include <DirectXCollision.h>

class Mesh {
public:
	struct LOD {
		uint64_t vertexBuffer;
		uint64_t indexBuffer;
		uint64_t vertexCount;
		uint64_t indexCount;
	};

	DirectX::BoundingBox boundingBox;

	uint32_t lodCount;
	uint32_t selectedLod;
	LOD lods[4];

	void AddLOD(const LOD & lod) {
		if(lodCount > 3) {
			return;
		}
		lods[lodCount] = lod;
		lodCount += 1;
	}

	void Draw(Egg::Graphics::IRenderContext * ctx) {
		if(lodCount == 0) {
			return;
		}

		LOD selected = lods[selectedLod];

		if(selected.indexCount > 0) {
			ctx->SetIndexBuffer(selected.indexBuffer);
			ctx->SetVertexBuffer(selected.vertexBuffer);
			ctx->DrawIndexed(selected.indexCount);
		} else {
			ctx->SetVertexBuffer(selected.vertexBuffer);
			ctx->Draw(selected.vertexCount);
		}
	}

};
