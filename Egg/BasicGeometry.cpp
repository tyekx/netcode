#include "BasicGeometry.h"

Egg::Mesh::Geometry::P Egg::BasicGeometry::CreateLine(ID3D12Device * device, const DirectX::XMFLOAT3 & color) {
	WireframeVertex vertices[2];

	vertices[0].position = DirectX::XMFLOAT3{ 0,0,0 };
	vertices[0].color = color;
	vertices[1].position = DirectX::XMFLOAT3{ 0,0,1.0f };
	vertices[1].color = color;

	Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, sizeof(vertices), sizeof(WireframeVertex));

	geom->SetTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	geom->AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	geom->AddInputElement({ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	return geom;
}

Egg::Mesh::Geometry::P Egg::BasicGeometry::CreateBoxWireframe(ID3D12Device * device, const DirectX::XMFLOAT3 & he, const DirectX::XMFLOAT3 & color) {
	WireframeVertex vertices[24];

	for(unsigned int i = 0; i < 24; ++i) {
		vertices[i].color = color;
	}

	unsigned int t = 0;
	vertices[t + 0].position = DirectX::XMFLOAT3{ -he.x, -he.y, -he.z };
	vertices[t + 1].position = DirectX::XMFLOAT3{ -he.x,  he.y, -he.z };

	vertices[t + 2].position = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };
	vertices[t + 3].position = DirectX::XMFLOAT3{ he.x,  he.y, -he.z };

	vertices[t + 4].position = DirectX::XMFLOAT3{ -he.x, -he.y, he.z };
	vertices[t + 5].position = DirectX::XMFLOAT3{ -he.x,  he.y, he.z };

	vertices[t + 6].position = DirectX::XMFLOAT3{ he.x, -he.y, he.z };
	vertices[t + 7].position = DirectX::XMFLOAT3{ he.x,  he.y, he.z };

	t += 8;

	vertices[t + 0].position = DirectX::XMFLOAT3{ -he.x, -he.y, -he.z };
	vertices[t + 1].position = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };

	vertices[t + 2].position = DirectX::XMFLOAT3{ he.x, -he.y, he.z };
	vertices[t + 3].position = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };

	vertices[t + 4].position = DirectX::XMFLOAT3{ -he.x, -he.y, he.z };
	vertices[t + 5].position = DirectX::XMFLOAT3{ he.x, -he.y, he.z };

	vertices[t + 6].position = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };
	vertices[t + 7].position = DirectX::XMFLOAT3{ he.x, -he.y, he.z };

	t += 8;

	vertices[t + 0].position = DirectX::XMFLOAT3{ -he.x, he.y, -he.z };
	vertices[t + 1].position = DirectX::XMFLOAT3{ he.x,  he.y, -he.z };

	vertices[t + 2].position = DirectX::XMFLOAT3{ -he.x,  he.y, he.z };
	vertices[t + 3].position = DirectX::XMFLOAT3{ -he.x,  he.y, -he.z };

	vertices[t + 4].position = DirectX::XMFLOAT3{ -he.x, he.y, he.z };
	vertices[t + 5].position = DirectX::XMFLOAT3{ he.x,  he.y, he.z };

	vertices[t + 6].position = DirectX::XMFLOAT3{ he.x,  he.y, -he.z };
	vertices[t + 7].position = DirectX::XMFLOAT3{ he.x,  he.y, he.z };


	Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, sizeof(vertices), sizeof(WireframeVertex));


	geom->SetTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	geom->AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	geom->AddInputElement({ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	return geom;
}

Egg::Mesh::Geometry::P Egg::BasicGeometry::CreatePlaneWireframe(ID3D12Device * device, unsigned int gridSectionCount, const DirectX::XMFLOAT3 & color) {
	unsigned int verticesLength = 2 * 2 * (gridSectionCount + 1);
	WireframeVertex * vertices = new WireframeVertex[verticesLength];
	float g = (float)gridSectionCount;

	for(unsigned int i = 0; i <= gridSectionCount; ++i) {
		vertices[2 * i].position = DirectX::XMFLOAT3{ 0, float(i) / g - 0.5f, -0.5f };
		vertices[2 * i].color = color;
		vertices[2 * i + 1].position = DirectX::XMFLOAT3{ 0, float(i) / g - 0.5f, 0.5f };
		vertices[2 * i + 1].color = color;

		vertices[2 * i + (2 * gridSectionCount + 2)].position = DirectX::XMFLOAT3{ 0, -0.5f, float(i) / g - 0.5f };
		vertices[2 * i + (2 * gridSectionCount + 2)].color = color;
		vertices[2 * i + (2 * gridSectionCount + 2) + 1].position = DirectX::XMFLOAT3{ 0, 0.5f, float(i) / g - 0.5f };
		vertices[2 * i + (2 * gridSectionCount + 2) + 1].color = color;
	}



	Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, verticesLength * sizeof(WireframeVertex), sizeof(WireframeVertex));

	delete[] vertices;

	geom->SetTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	geom->AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	geom->AddInputElement({ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	return geom;
}

Egg::Mesh::Geometry::P Egg::BasicGeometry::CreateCapsuleWireframe(ID3D12Device * device, float height, float radius, const DirectX::XMFLOAT3 & color) {

	WireframeVertex vertices[108];

	float h = height;
	float r = radius;
	constexpr static float PI = 3.1415925f;
	constexpr static float PI2 = 2.0f * PI;

	for(unsigned int i = 0; i < 12; ++i) {

		float arg = float(i) * PI2 / 12.0f;
		float argEnd = float(i + 1) * PI2 / 12.0f;

		DirectX::XMFLOAT3 lineStart{ h / 2.0f, r * cosf(arg), r * sinf(arg) };
		DirectX::XMFLOAT3 lineEnd{ h / 2.0f, r * cosf(argEnd), r * sinf(argEnd) };

		vertices[2 * i].position = lineStart;
		vertices[2 * i].color = color;
		vertices[2 * i + 1].position = lineEnd;
		vertices[2 * i + 1].color = color;

		lineStart.x = -lineStart.x;
		lineEnd.x = -lineEnd.x;

		vertices[2 * i + 26].position = lineStart;
		vertices[2 * i + 26].color = color;
		vertices[2 * i + 27].position = lineEnd;
		vertices[2 * i + 27].color = color;

	}

	vertices[24] = vertices[23];
	vertices[25] = vertices[0];

	vertices[50] = vertices[49];
	vertices[51] = vertices[26];

	for(unsigned int i = 0; i < 6; ++i) {

		float arg = float(i) * PI / 6.0f;
		float argEnd = float(i + 1) * PI / 6.0f;

		DirectX::XMFLOAT3 lineStart{ (h / 2.0f) + r * sinf(arg), 0,  r * cosf(arg) };
		DirectX::XMFLOAT3 lineEnd{ (h / 2.0f) + r * sinf(argEnd), 0, r * cosf(argEnd) };

		vertices[2 * i + 52].position = lineStart;
		vertices[2 * i + 52].color = color;

		vertices[2 * i + 53].position = lineEnd;
		vertices[2 * i + 53].color = color;

		lineStart = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(arg), 0, r * cosf(arg) };
		lineEnd = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(argEnd), 0, r * cosf(argEnd) };

		vertices[2 * i + 64].position = lineStart;
		vertices[2 * i + 64].color = color;

		vertices[2 * i + 65].position = lineEnd;
		vertices[2 * i + 65].color = color;

		lineStart = DirectX::XMFLOAT3{(h / 2.0f) + r * sinf(arg), r * cosf(arg),  0 };
		lineEnd = DirectX::XMFLOAT3{  (h / 2.0f) + r * sinf(argEnd), r * cosf(argEnd),0 };


		vertices[2 * i + 76].position = lineStart;
		vertices[2 * i + 76].color = color;

		vertices[2 * i + 77].position = lineEnd;
		vertices[2 * i + 77].color = color;

		lineStart = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(arg), r * cosf(arg), 0 };
		lineEnd = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(argEnd),r * cosf(argEnd),  0 };


		vertices[2 * i + 88].position = lineStart;
		vertices[2 * i + 88].color = color;

		vertices[2 * i + 89].position = lineEnd;
		vertices[2 * i + 89].color = color;
	}

	DirectX::XMFLOAT3 lineSt{ h / 2.0f, r, 0 };
	DirectX::XMFLOAT3 lineNd{ -h / 2.0f, r, 0 };

	vertices[100].position = lineSt;
	vertices[100].color = color;

	vertices[101].position = lineNd;
	vertices[101].color = color;

	lineSt = DirectX::XMFLOAT3{ h / 2.0f, -r, 0 };
	lineNd = DirectX::XMFLOAT3{ -h / 2.0f , -r,0 };


	vertices[102].position = lineSt;
	vertices[102].color = color;

	vertices[103].position = lineNd;
	vertices[103].color = color;

	lineSt = DirectX::XMFLOAT3{ h / 2.0f, 0,  r};
	lineNd = DirectX::XMFLOAT3{ -h / 2.0f, 0,  r };

	vertices[104].position = lineSt;
	vertices[104].color = color;

	vertices[105].position = lineNd;
	vertices[105].color = color;

	lineSt = DirectX::XMFLOAT3{ h / 2.0f, 0, -r };
	lineNd = DirectX::XMFLOAT3{ -h / 2.0f, 0, -r };

	vertices[106].position = lineSt;
	vertices[106].color = color;

	vertices[107].position = lineNd;
	vertices[107].color = color;

	Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, sizeof(vertices), sizeof(WireframeVertex));

	geom->SetTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	geom->AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	geom->AddInputElement({ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	return geom;
}

Egg::Mesh::Geometry::P Egg::BasicGeometry::CreateBox(ID3D12Device * device) {
	PNT_Vertex vertices[] = {
		{ { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 1.0f } },
	{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 1.0f } },
	{ { 0.5f,  0.5f, -0.5f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 1.0f } },

	{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f,  0.0f },{ 0.0f, 0.0f } },
	{ { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { 0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f,  0.0f },{ 1.0f, 1.0f } },

	{ { 0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f,  0.0f },{ 0.0f, 0.0f } },
	{ { 0.5f,  0.5f, -0.5f },{ 1.0f, 0.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { 0.5f, -0.5f, 0.5f },{ 1.0f, 0.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.5f },{ 1.0f, 0.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f,  0.5f, -0.5f },{ 1.0f, 0.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { 0.5f,  0.5f, 0.5f },{ 1.0f, 0.0f,  0.0f },{ 1.0f, 1.0f } },

	{ { 0.5f, -0.5f, 0.5f },{ 0.0f, 0.0f,  1.0f },{ 0.0f, 0.0f } },
	{ { 0.5f,  0.5f, 0.5f },{ 0.0f, 0.0f,  1.0f },{ 0.0f, 1.0f } },
	{ { -0.5f, -0.5f, 0.5f },{ 0.0f, 0.0f,  1.0f },{ 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, 0.5f },{ 0.0f, 0.0f,  1.0f },{ 1.0f, 0.0f } },
	{ { 0.5f,  0.5f, 0.5f },{ 0.0f, 0.0f,  1.0f },{ 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, 0.5f },{ 0.0f, 0.0f,  1.0f },{ 1.0f, 1.0f } },

	{ { -0.5f, -0.5f, 0.5f },{ 0.0f, -1.0f,  0.0f },{ 0.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f },{ 0.0f, -1.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { 0.5f, -0.5f, 0.5f },{ 0.0f, -1.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { 0.5f, -0.5f, 0.5f },{ 0.0f, -1.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f },{ 0.0f, -1.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { 0.5f, -0.5f, -0.5f },{ 0.0f, -1.0f,  0.0f },{ 1.0f, 1.0f } },

	{ { -0.5f, -0.5f, 0.5f },{ -1.0f, 0.0f,  0.0f },{ 0.0f, 0.0f } },
	{ { -0.5f,  0.5f, 0.5f },{ -1.0f, 0.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { -0.5f, -0.5f, -0.5f },{ -1.0f, 0.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f },{ -1.0f, 0.0f,  0.0f },{ 1.0f, 0.0f } },
	{ { -0.5f,  0.5f, 0.5f },{ -1.0f, 0.0f,  0.0f },{ 0.0f, 1.0f } },
	{ { -0.5f,  0.5f, -0.5f },{ -1.0f, 0.0f,  0.0f },{ 1.0f, 1.0f } }
	};
	//ID3D12Device * device, void * data, unsigned int sizeInBytes, unsigned int stride
	Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, sizeof(vertices), sizeof(Egg::PNT_Vertex));

	geom->SetVertexType(Egg::PNT_Vertex::type);

	return geom;
}
