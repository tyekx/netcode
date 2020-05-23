#include "BasicGeometry.h"
#include <map>
#include <memory>

namespace Netcode::Graphics {

	class VertexDataIterator {
		unsigned char * data;
		unsigned int stride;
		unsigned int positionDataOffset;
		uint32_t inserted;
	public:
		VertexDataIterator(void * data, unsigned int stride, unsigned int positionDataOffset) :
			data{ reinterpret_cast<unsigned char *>(data) },
			stride{ stride },
			positionDataOffset{ positionDataOffset },
			inserted{ 0 } {

		}

		uint32_t Add(const Netcode::Float3 & v) {
			uint32_t idx = inserted++;
			(*this)[idx] = v;
			return idx;
		}

		Netcode::Float3 & operator[](unsigned int index) {
			return *(reinterpret_cast<Netcode::Float3*>(data + stride * index + positionDataOffset));
		}
	};

	void BasicGeometry::CreateLine(void * dstData, unsigned int stride, unsigned int positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		for(unsigned int i = 0; i < 2; ++i) {
			vertices[i] = Netcode::Float3{ 0, 0, static_cast<float>(i) };
		}
	}


	void BasicGeometry::CreateBoxWireframe(_Out_writes_(24 * stride) void * dstData, unsigned int stride, const Netcode::Float3 & he, unsigned int positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		unsigned int t = 0;
		vertices[t + 0] = Netcode::Float3{ -he.x, -he.y, -he.z };
		vertices[t + 1] = Netcode::Float3{ -he.x,  he.y, -he.z };

		vertices[t + 2] = Netcode::Float3{ he.x, -he.y, -he.z };
		vertices[t + 3] = Netcode::Float3{ he.x,  he.y, -he.z };

		vertices[t + 4] = Netcode::Float3{ -he.x, -he.y, he.z };
		vertices[t + 5] = Netcode::Float3{ -he.x,  he.y, he.z };

		vertices[t + 6] = Netcode::Float3{ he.x, -he.y, he.z };
		vertices[t + 7] = Netcode::Float3{ he.x,  he.y, he.z };

		t += 8;

		vertices[t + 0] = Netcode::Float3{ -he.x, -he.y, -he.z };
		vertices[t + 1] = Netcode::Float3{ he.x, -he.y, -he.z };

		vertices[t + 2] = Netcode::Float3{ he.x, -he.y, he.z };
		vertices[t + 3] = Netcode::Float3{ he.x, -he.y, -he.z };

		vertices[t + 4] = Netcode::Float3{ -he.x, -he.y, he.z };
		vertices[t + 5] = Netcode::Float3{ he.x, -he.y, he.z };

		vertices[t + 6] = Netcode::Float3{ -he.x, -he.y, -he.z };
		vertices[t + 7] = Netcode::Float3{ -he.x, -he.y, he.z };

		t += 8;

		vertices[t + 0] = Netcode::Float3{ -he.x, he.y, -he.z };
		vertices[t + 1] = Netcode::Float3{ he.x,  he.y, -he.z };

		vertices[t + 2] = Netcode::Float3{ -he.x,  he.y, he.z };
		vertices[t + 3] = Netcode::Float3{ -he.x,  he.y, -he.z };

		vertices[t + 4] = Netcode::Float3{ -he.x, he.y, he.z };
		vertices[t + 5] = Netcode::Float3{ he.x,  he.y, he.z };

		vertices[t + 6] = Netcode::Float3{ he.x,  he.y, -he.z };
		vertices[t + 7] = Netcode::Float3{ he.x,  he.y, he.z };
	}

	void BasicGeometry::CreatePlaneWireframe(_Out_writes_(4 * (gridSectionCount + 1) * stride) void * dstData, unsigned int stride, unsigned int gridSectionCount, unsigned int positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);
		const float g = static_cast<float>(gridSectionCount);
		for(unsigned int i = 0; i <= gridSectionCount; ++i) {
			vertices[2 * i] = Netcode::Float3{ 0, static_cast<float>(i) / g - 0.5f, -0.5f };
			vertices[2 * i + 1] = Netcode::Float3{ 0,  static_cast<float>(i) / g - 0.5f, 0.5f };

			vertices[2 * i + (2 * gridSectionCount + 2)] = Netcode::Float3{ 0, -0.5f,  static_cast<float>(i) / g - 0.5f };
			vertices[2 * i + (2 * gridSectionCount + 2) + 1] = Netcode::Float3{ 0, 0.5f,  static_cast<float>(i) / g - 0.5f };
		}
	}

	void BasicGeometry::CreateCapsuleWireframe(_Out_writes_(108 * stride) void * dstData, unsigned int stride, const Netcode::Float2 & args, unsigned int positionDataOffset)
	{
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		float h = args.x;
		float r = args.y;
		constexpr static float PI = DirectX::XM_PI;
		constexpr static float PI2 = DirectX::XM_2PI;

		for(unsigned int i = 0; i < 12; ++i) {
			float arg = float(i) * PI2 / 12.0f;
			float argEnd = float(i + 1) * PI2 / 12.0f;

			vertices[2 * i] = Netcode::Float3 { h / 2.0f, r * cosf(arg), r * sinf(arg) };
			vertices[2 * i + 1] = Netcode::Float3 { h / 2.0f, r * cosf(argEnd), r * sinf(argEnd) };

			vertices[2 * i + 26] = Netcode::Float3{ -h / 2.0f, r * cosf(arg), r * sinf(arg) };
			vertices[2 * i + 27] = Netcode::Float3{ -h / 2.0f, r * cosf(argEnd), r * sinf(argEnd) };
		}

		vertices[24] = vertices[23];
		vertices[25] = vertices[0];

		vertices[50] = vertices[49];
		vertices[51] = vertices[26];

		for(unsigned int i = 0; i < 6; ++i) {
			float arg = float(i) * PI / 6.0f;
			float argEnd = float(i + 1) * PI / 6.0f;

			vertices[2 * i + 52] = Netcode::Float3 { (h / 2.0f) + r * sinf(arg), 0,  r * cosf(arg) };;
			vertices[2 * i + 53] = Netcode::Float3 { (h / 2.0f) + r * sinf(argEnd), 0, r * cosf(argEnd) };

			vertices[2 * i + 64] = Netcode::Float3{ (-h / 2.0f) - r * sinf(arg), 0, r * cosf(arg) };
			vertices[2 * i + 65] = Netcode::Float3{ (-h / 2.0f) - r * sinf(argEnd), 0, r * cosf(argEnd) };

			vertices[2 * i + 76] = Netcode::Float3{ (h / 2.0f) + r * sinf(arg), r * cosf(arg),  0 };
			vertices[2 * i + 77] = Netcode::Float3{ (h / 2.0f) + r * sinf(argEnd), r * cosf(argEnd),0 };

			vertices[2 * i + 88] = Netcode::Float3{ (-h / 2.0f) - r * sinf(arg), r * cosf(arg), 0 };
			vertices[2 * i + 89] = Netcode::Float3{ (-h / 2.0f) - r * sinf(argEnd),r * cosf(argEnd),  0 };
		}

		vertices[100] = Netcode::Float3 { h / 2.0f, r, 0 };
		vertices[101] = Netcode::Float3 { -h / 2.0f, r, 0 };

		vertices[102] = Netcode::Float3{ h / 2.0f, -r, 0 };
		vertices[103] = Netcode::Float3{ -h / 2.0f , -r,0 };

		vertices[104] = Netcode::Float3{ h / 2.0f, 0,  r };
		vertices[105] = Netcode::Float3{ -h / 2.0f, 0,  r };

		vertices[106] = Netcode::Float3{ h / 2.0f, 0, -r };
		vertices[107] = Netcode::Float3{ -h / 2.0f, 0, -r };
	}

	static std::map<uint64_t, uint32_t> middlePointIndexCache;

	static uint32_t AddVertex(VertexDataIterator & vIter, const Netcode::Float3 & position) {
		DirectX::XMVECTOR middleV = DirectX::XMLoadFloat3(&position);
		middleV = DirectX::XMVector3Normalize(middleV);
		Netcode::Float3 pos;
		DirectX::XMStoreFloat3(&pos, middleV);

		return vIter.Add(pos);
	}

	static uint32_t GetMiddlePoint(uint32_t p1, uint32_t p2, VertexDataIterator & vIter)
	{
		// first check if we have it already
		bool firstIsSmaller = p1 < p2;
		uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
		uint64_t greaterIndex = firstIsSmaller ? p2 : p1;
		uint64_t key = (smallerIndex << 32) + greaterIndex;

		const auto it = middlePointIndexCache.find(key);
		if(it != middlePointIndexCache.end())
		{
			return it->second;
		}

		// not in cache, calculate it
		Netcode::Float3 point1 = vIter[p1];
		Netcode::Float3 point2 = vIter[p2];
		Netcode::Float3 middle = Netcode::Float3(
			(point1.x + point2.x) / 2.0f,
			(point1.y + point2.y) / 2.0f,
			(point1.z + point2.z) / 2.0f);

		uint32_t idx = AddVertex(vIter, middle);

		middlePointIndexCache[key] = idx;
		return idx;
	}


	void BasicGeometry::CreateSphereWireFrame(void * dstData, uint32_t stride, float radius, uint32_t positionDataOffset) {
		float r = radius; middlePointIndexCache.clear();

		std::unique_ptr<Netcode::Float3[]> tempVertexData = std::make_unique<Netcode::Float3[]>(162);

		VertexDataIterator tempVertexDataIter{ tempVertexData.get(), sizeof(Netcode::Float3), 0 };

		VertexDataIterator vDataIter{ dstData, stride, positionDataOffset };

		float t = (1.0f + sqrtf(5.0f)) / 2.0f;

		// v 12
		// v 42
		// v 162

		// f 320

		// e 480


		AddVertex(tempVertexDataIter, Netcode::Float3(-1, t, 0));
		AddVertex(tempVertexDataIter, Netcode::Float3(1, t, 0));
		AddVertex(tempVertexDataIter, Netcode::Float3(-1, -t, 0));
		AddVertex(tempVertexDataIter, Netcode::Float3(1, -t, 0));

		AddVertex(tempVertexDataIter, Netcode::Float3(0, -1, t));
		AddVertex(tempVertexDataIter, Netcode::Float3(0, 1, t));
		AddVertex(tempVertexDataIter, Netcode::Float3(0, -1, -t));
		AddVertex(tempVertexDataIter, Netcode::Float3(0, 1, -t));

		AddVertex(tempVertexDataIter, Netcode::Float3(t, 0, -1));
		AddVertex(tempVertexDataIter, Netcode::Float3(t, 0, 1));
		AddVertex(tempVertexDataIter, Netcode::Float3(-t, 0, -1));
		AddVertex(tempVertexDataIter, Netcode::Float3(-t, 0, 1));

		struct Face {
			uint32_t idx0;
			uint32_t idx1;
			uint32_t idx2;
		};

		std::vector<Face> faces;

		// 5 faces around point 0
		faces.push_back(Face{ 0, 11, 5 });
		faces.push_back(Face{ 0, 5, 1 });
		faces.push_back(Face{ 0, 1, 7 });
		faces.push_back(Face{ 0, 7, 10 });
		faces.push_back(Face{ 0, 10, 11 });

		// 5 adjacent faces
		faces.push_back(Face{ 1, 5, 9 });
		faces.push_back(Face{ 5, 11, 4 });
		faces.push_back(Face{ 11, 10, 2 });
		faces.push_back(Face{ 10, 7, 6 });
		faces.push_back(Face{ 7, 1, 8 });

		// 5 faces around point 3
		faces.push_back(Face{ 3, 9, 4 });
		faces.push_back(Face{ 3, 4, 2 });
		faces.push_back(Face{ 3, 2, 6 });
		faces.push_back(Face{ 3, 6, 8 });
		faces.push_back(Face{ 3, 8, 9 });

		// 5 adjacent faces
		faces.push_back(Face{ 4, 9, 5 });
		faces.push_back(Face{ 2, 4, 11 });
		faces.push_back(Face{ 6, 2, 10 });
		faces.push_back(Face{ 8, 6, 7 });
		faces.push_back(Face{ 9, 8, 1 });

		for(int i = 0; i < 2; i++)
		{
			std::vector<Face> faces2;
			for(const auto & tri : faces) {
				// replace triangle by 4 triangles
				uint32_t a = GetMiddlePoint(tri.idx0, tri.idx1, tempVertexDataIter);
				uint32_t b = GetMiddlePoint(tri.idx1, tri.idx2, tempVertexDataIter);
				uint32_t c = GetMiddlePoint(tri.idx2, tri.idx0, tempVertexDataIter);

				faces2.push_back(Face{ tri.idx0, a, c });
				faces2.push_back(Face{ tri.idx1, b, a });
				faces2.push_back(Face{ tri.idx2, c, b });
				faces2.push_back(Face{ a, b, c });
			}
			faces = std::move(faces2);
		}

		for(const Face & face : faces) {
			Netcode::Float3 p0 = tempVertexDataIter[face.idx0];
			Netcode::Float3 p1 = tempVertexDataIter[face.idx1];
			Netcode::Float3 p2 = tempVertexDataIter[face.idx2];
			p0 = Netcode::Float3{ r * p0.x, r * p0.y, r * p0.z };
			p1 = Netcode::Float3{ r * p1.x, r * p1.y, r * p1.z };
			p2 = Netcode::Float3{ r * p2.x, r * p2.y, r * p2.z };

			vDataIter.Add(p0);
			vDataIter.Add(p1);

			vDataIter.Add(p1);
			vDataIter.Add(p2);

			vDataIter.Add(p2);
			vDataIter.Add(p0);
		}

	}
}
