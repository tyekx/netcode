#pragma once

#include "Common.h"
#include "ConstantBufferTypes.h"

namespace Egg {

	class Mesh {
	public:
		D3D12_INDEX_BUFFER_VIEW ibv;
		D3D12_VERTEX_BUFFER_VIEW vbv;
		D3D12_PRIMITIVE_TOPOLOGY topology;
		UINT indexCount;
		UINT vertexCount;
		PerMeshCb * perMeshCb;

		inline void Draw(ID3D12GraphicsCommandList * gcl) {
			gcl->IASetPrimitiveTopology(topology);
			gcl->IASetVertexBuffers(0, 1, &vbv);

			if(indexCount == 0) {
				gcl->DrawInstanced(vertexCount, 1, 0, 0);
			} else {
				gcl->IASetIndexBuffer(&ibv);
				gcl->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
			}
		}
	};

}
