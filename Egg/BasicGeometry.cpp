#include "BasicGeometry.h"

namespace Egg::Graphics {

	class VertexDataIterator {
		unsigned char * data;
		unsigned int stride;
		unsigned int positionDataOffset;
	public:
		VertexDataIterator(void * data, unsigned int stride, unsigned int positionDataOffset) :
			data{ reinterpret_cast<unsigned char *>(data) },
			stride{ stride },
			positionDataOffset{ positionDataOffset }
		{

		}

		DirectX::XMFLOAT3 & operator[](unsigned int index) {
			return *(reinterpret_cast<DirectX::XMFLOAT3*>(data + stride * index + positionDataOffset));
		}
	};

	void BasicGeometry::CreateLine(void * dstData, unsigned int stride, unsigned int positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		for(unsigned int i = 0; i < 2; ++i) {
			vertices[i] = DirectX::XMFLOAT3{ 0, 0, static_cast<float>(i) };
		}
	}


	void BasicGeometry::CreateBoxWireframe(_Out_writes_(24 * stride) void * dstData, unsigned int stride, const DirectX::XMFLOAT3 & he, unsigned int positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		unsigned int t = 0;
		vertices[t + 0] = DirectX::XMFLOAT3{ -he.x, -he.y, -he.z };
		vertices[t + 1] = DirectX::XMFLOAT3{ -he.x,  he.y, -he.z };

		vertices[t + 2] = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };
		vertices[t + 3] = DirectX::XMFLOAT3{ he.x,  he.y, -he.z };

		vertices[t + 4] = DirectX::XMFLOAT3{ -he.x, -he.y, he.z };
		vertices[t + 5] = DirectX::XMFLOAT3{ -he.x,  he.y, he.z };

		vertices[t + 6] = DirectX::XMFLOAT3{ he.x, -he.y, he.z };
		vertices[t + 7] = DirectX::XMFLOAT3{ he.x,  he.y, he.z };

		t += 8;

		vertices[t + 0] = DirectX::XMFLOAT3{ -he.x, -he.y, -he.z };
		vertices[t + 1] = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };

		vertices[t + 2] = DirectX::XMFLOAT3{ he.x, -he.y, he.z };
		vertices[t + 3] = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };

		vertices[t + 4] = DirectX::XMFLOAT3{ -he.x, -he.y, he.z };
		vertices[t + 5] = DirectX::XMFLOAT3{ he.x, -he.y, he.z };

		vertices[t + 6] = DirectX::XMFLOAT3{ he.x, -he.y, -he.z };
		vertices[t + 7] = DirectX::XMFLOAT3{ he.x, -he.y, he.z };

		t += 8;

		vertices[t + 0] = DirectX::XMFLOAT3{ -he.x, he.y, -he.z };
		vertices[t + 1] = DirectX::XMFLOAT3{ he.x,  he.y, -he.z };

		vertices[t + 2] = DirectX::XMFLOAT3{ -he.x,  he.y, he.z };
		vertices[t + 3] = DirectX::XMFLOAT3{ -he.x,  he.y, -he.z };

		vertices[t + 4] = DirectX::XMFLOAT3{ -he.x, he.y, he.z };
		vertices[t + 5] = DirectX::XMFLOAT3{ he.x,  he.y, he.z };

		vertices[t + 6] = DirectX::XMFLOAT3{ he.x,  he.y, -he.z };
		vertices[t + 7] = DirectX::XMFLOAT3{ he.x,  he.y, he.z };
	}

	void BasicGeometry::CreatePlaneWireframe(_Out_writes_(4 * (gridSectionCount + 1) * stride) void * dstData, unsigned int stride, unsigned int gridSectionCount, unsigned int positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);
		const float g = static_cast<float>(gridSectionCount);
		for(unsigned int i = 0; i <= gridSectionCount; ++i) {
			vertices[2 * i] = DirectX::XMFLOAT3{ 0, static_cast<float>(i) / g - 0.5f, -0.5f };
			vertices[2 * i + 1] = DirectX::XMFLOAT3{ 0,  static_cast<float>(i) / g - 0.5f, 0.5f };

			vertices[2 * i + (2 * gridSectionCount + 2)] = DirectX::XMFLOAT3{ 0, -0.5f,  static_cast<float>(i) / g - 0.5f };
			vertices[2 * i + (2 * gridSectionCount + 2) + 1] = DirectX::XMFLOAT3{ 0, 0.5f,  static_cast<float>(i) / g - 0.5f };
		}
	}

	void BasicGeometry::CreateCapsuleWireframe(_Out_writes_(108 * stride) void * dstData, unsigned int stride, float height, float radius, unsigned int positionDataOffset)
	{
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		float h = height;
		float r = radius;
		constexpr static float PI = 3.1415925f;
		constexpr static float PI2 = 2.0f * PI;

		for(unsigned int i = 0; i < 12; ++i) {
			float arg = float(i) * PI2 / 12.0f;
			float argEnd = float(i + 1) * PI2 / 12.0f;

			vertices[2 * i] = DirectX::XMFLOAT3 { h / 2.0f, r * cosf(arg), r * sinf(arg) };
			vertices[2 * i + 1] = DirectX::XMFLOAT3 { h / 2.0f, r * cosf(argEnd), r * sinf(argEnd) };

			vertices[2 * i + 26] = DirectX::XMFLOAT3{ -h / 2.0f, r * cosf(arg), r * sinf(arg) };;
			vertices[2 * i + 27] = DirectX::XMFLOAT3{ -h / 2.0f, r * cosf(argEnd), r * sinf(argEnd) };;
		}

		vertices[24] = vertices[23];
		vertices[25] = vertices[0];

		vertices[50] = vertices[49];
		vertices[51] = vertices[26];

		for(unsigned int i = 0; i < 6; ++i) {
			float arg = float(i) * PI / 6.0f;
			float argEnd = float(i + 1) * PI / 6.0f;

			vertices[2 * i + 52] = DirectX::XMFLOAT3 { (h / 2.0f) + r * sinf(arg), 0,  r * cosf(arg) };;
			vertices[2 * i + 53] = DirectX::XMFLOAT3 { (h / 2.0f) + r * sinf(argEnd), 0, r * cosf(argEnd) };

			vertices[2 * i + 64] = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(arg), 0, r * cosf(arg) };
			vertices[2 * i + 65] = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(argEnd), 0, r * cosf(argEnd) };

			vertices[2 * i + 76] = DirectX::XMFLOAT3{ (h / 2.0f) + r * sinf(arg), r * cosf(arg),  0 };
			vertices[2 * i + 77] = DirectX::XMFLOAT3{ (h / 2.0f) + r * sinf(argEnd), r * cosf(argEnd),0 };

			vertices[2 * i + 88] = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(arg), r * cosf(arg), 0 };
			vertices[2 * i + 89] = DirectX::XMFLOAT3{ (-h / 2.0f) - r * sinf(argEnd),r * cosf(argEnd),  0 };
		}

		vertices[100] = DirectX::XMFLOAT3 { h / 2.0f, r, 0 };
		vertices[101] = DirectX::XMFLOAT3 { -h / 2.0f, r, 0 };

		vertices[102] = DirectX::XMFLOAT3{ h / 2.0f, -r, 0 };
		vertices[103] = DirectX::XMFLOAT3{ -h / 2.0f , -r,0 };

		vertices[104] = DirectX::XMFLOAT3{ h / 2.0f, 0,  r };
		vertices[105] = DirectX::XMFLOAT3{ -h / 2.0f, 0,  r };

		vertices[106] = DirectX::XMFLOAT3{ h / 2.0f, 0, -r };
		vertices[107] = DirectX::XMFLOAT3{ -h / 2.0f, 0, -r };
	}

}
