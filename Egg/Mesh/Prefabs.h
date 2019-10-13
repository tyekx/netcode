#pragma once

#include "Geometry.h"
#include "../Vertex.h"

namespace Egg {
	namespace Mesh {

		class Prefabs {
			Prefabs() = delete;
			~Prefabs() = delete;
		public:

			static Egg::Mesh::Geometry::P IndexedFullscreenQuad(ID3D12Device * device) {
				PNT_Vertex vertices[] = {
					{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
					{ {  1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
					{ { -1.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
					{ { 1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f}, { 1.0f, 0.0f } }
				};

				unsigned int indices[] = {
					0, 1, 2, 
					2, 1, 3
				};

				Egg::Mesh::Geometry::P geometry = Egg::Mesh::IndexedGeometry::Create(device, vertices, (unsigned int)sizeof(vertices), (unsigned int)sizeof(PNT_Vertex),
																							 indices, (unsigned int)sizeof(indices), DXGI_FORMAT_R32_UINT);

				geometry->SetVertexType(PNT_Vertex::type);

				return geometry;
			}

			static Egg::Mesh::Geometry::P FullScreenQuad(ID3D12Device * device) {
				PNT_Vertex vertices[] = {
					{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
					{ {  1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
					{ { -1.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
					{ { -1.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
					{ {  1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },
					{ { 1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f}, { 1.0f, 0.0f } }
				}; 


				unsigned int vertexBufferSize = (unsigned int)sizeof(vertices);
				Egg::Mesh::Geometry::P geometry = Egg::Mesh::VertexStreamGeometry::Create(device, reinterpret_cast<void*>(vertices), vertexBufferSize, (unsigned int)sizeof(PNT_Vertex));

				geometry->SetVertexType(PNT_Vertex::type);

				return geometry;
			}

			static Egg::Mesh::Geometry::P UnitBox(ID3D12Device * device) {
				PNT_Vertex vertices[] = {
					{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
					{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
					{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
					{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
					{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
					{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },

					{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f,  0.0f }, { 0.0f, 0.0f } },
					{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f,  0.0f }, { 1.0f, 0.0f } },
					{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f,  0.0f }, { 1.0f, 0.0f } },
					{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f,  0.0f }, { 1.0f, 1.0f } },

					{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f,  0.0f }, { 0.0f, 0.0f } },
					{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
					{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
					{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  0.5f,  0.5f, 0.5f }, { 1.0f, 0.0f,  0.0f }, { 1.0f, 1.0f } },

					{ {  0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
					{ {  0.5f,  0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 1.0f } },
					{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 1.0f, 0.0f } },
					{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 1.0f, 0.0f } },
					{ {  0.5f,  0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 1.0f } },
					{ { -0.5f,  0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 1.0f, 1.0f } },

					{ { -0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
					{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
					{ {  0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
					{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },

					{ {  -0.5f, -0.5f, 0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 0.0f } },
					{ {  -0.5f,  0.5f, 0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
					{ {  -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
					{ {  -0.5f,  0.5f, 0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
					{ {  -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 1.0f } }
				};


				unsigned int vertexBufferSize = sizeof(vertices);
				Egg::Mesh::Geometry::P geometry = Egg::Mesh::VertexStreamGeometry::Create(device, reinterpret_cast<void*>(vertices), vertexBufferSize, (unsigned int)sizeof(PNT_Vertex));
				
				geometry->SetVertexType(PNT_Vertex::type);

				return geometry;
			}

		};

	}
}
