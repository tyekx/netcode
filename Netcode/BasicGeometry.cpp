#include "BasicGeometry.h"
#include <map>
#include <memory>

namespace Netcode::Graphics {

	class VertexDataIterator {
		uint8_t * data;
		uint32_t stride;
		uint32_t positionDataOffset;
		uint32_t inserted;
	public:
		VertexDataIterator(void * data, uint32_t stride, uint32_t positionDataOffset) :
			data{ reinterpret_cast<uint8_t *>(data) },
			stride{ stride },
			positionDataOffset{ positionDataOffset },
			inserted{ 0 } {

		}

		uint32_t Add(const Netcode::Float3 & v) {
			uint32_t idx = inserted++;
			(*this)[idx] = v;
			return idx;
		}

		Netcode::Float3 & operator[](uint32_t index) {
			return *(reinterpret_cast<Netcode::Float3*>(data + stride * index + positionDataOffset));
		}
	};

	uint32_t BasicGeometry::CreateLine(void * dstData, uint32_t stride, uint32_t positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		for(uint32_t i = 0; i < 2; ++i) {
			vertices[i] = Netcode::Float3{ 0, 0, static_cast<float>(i) };
		}

		return 2;
	}


	uint32_t BasicGeometry::CreateBoxWireframe(void * dstData, uint32_t stride, const Netcode::Float3 & he, uint32_t positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		uint32_t t = 0;
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

		return t + 8;
	}

	uint32_t BasicGeometry::CreatePlaneWireframe(void * dstData, uint32_t stride, uint32_t gridSectionCount, uint32_t positionDataOffset) {
		VertexDataIterator vertices(dstData, stride, positionDataOffset);
		const float g = static_cast<float>(gridSectionCount);
		for(uint32_t i = 0; i <= gridSectionCount; ++i) {
			vertices[2 * i] = Netcode::Float3{ 0, static_cast<float>(i) / g - 0.5f, -0.5f };
			vertices[2 * i + 1] = Netcode::Float3{ 0,  static_cast<float>(i) / g - 0.5f, 0.5f };

			vertices[2 * i + (2 * gridSectionCount + 2)] = Netcode::Float3{ 0, -0.5f,  static_cast<float>(i) / g - 0.5f };
			vertices[2 * i + (2 * gridSectionCount + 2) + 1] = Netcode::Float3{ 0, 0.5f,  static_cast<float>(i) / g - 0.5f };
		}

		return gridSectionCount + (2 * gridSectionCount + 2) + 2;
	}

	uint32_t BasicGeometry::GetCapsuleWireframeSize(uint32_t numSlices)
	{
		return numSlices * 4 + (numSlices / 2) * 8 + 8;
	}

	uint32_t BasicGeometry::CreateCapsuleWireframe(void * dstData, uint32_t stride, uint32_t numSlices, float radius, float halfHeight, uint32_t positionDataOffset)
	{
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		float h = halfHeight;
		float r = radius;

		float fNumSlices = static_cast<float>(numSlices);
		constexpr float PI2 = DirectX::XM_2PI;

		uint32_t vi = 0;
		for(uint32_t i = 0; i < numSlices; ++i) {
			float arg = float(i) * PI2 / fNumSlices;
			float argEnd = float(i + 1) * PI2 / fNumSlices;

			vertices[vi++] = Netcode::Float3{ h / 2.0f, r * cosf(arg), r * sinf(arg) };
			vertices[vi++] = Netcode::Float3{ h / 2.0f, r * cosf(argEnd), r * sinf(argEnd) };

			vertices[vi++] = Netcode::Float3{ -h / 2.0f, r * cosf(arg), r * sinf(arg) };
			vertices[vi++] = Netcode::Float3{ -h / 2.0f, r * cosf(argEnd), r * sinf(argEnd) };
		}

		for(uint32_t i = 0; i < (numSlices / 2); ++i) {
			float arg = float(i) * PI2 / fNumSlices;
			float argEnd = float(i + 1) * PI2 / fNumSlices;

			vertices[vi++] = Netcode::Float3{ (h / 2.0f) + r * sinf(arg), 0,  r * cosf(arg) };;
			vertices[vi++] = Netcode::Float3{ (h / 2.0f) + r * sinf(argEnd), 0, r * cosf(argEnd) };

			vertices[vi++] = Netcode::Float3{ (-h / 2.0f) - r * sinf(arg), 0, r * cosf(arg) };
			vertices[vi++] = Netcode::Float3{ (-h / 2.0f) - r * sinf(argEnd), 0, r * cosf(argEnd) };

			vertices[vi++] = Netcode::Float3{ (h / 2.0f) + r * sinf(arg), r * cosf(arg),  0 };
			vertices[vi++] = Netcode::Float3{ (h / 2.0f) + r * sinf(argEnd), r * cosf(argEnd),0 };

			vertices[vi++] = Netcode::Float3{ (-h / 2.0f) - r * sinf(arg), r * cosf(arg), 0 };
			vertices[vi++] = Netcode::Float3{ (-h / 2.0f) - r * sinf(argEnd),r * cosf(argEnd),  0 };
		}

		vertices[vi++] = Netcode::Float3{ h / 2.0f, r, 0 };
		vertices[vi++] = Netcode::Float3{ -h / 2.0f, r, 0 };

		vertices[vi++] = Netcode::Float3{ h / 2.0f, -r, 0 };
		vertices[vi++] = Netcode::Float3{ -h / 2.0f , -r,0 };

		vertices[vi++] = Netcode::Float3{ h / 2.0f, 0,  r };
		vertices[vi++] = Netcode::Float3{ -h / 2.0f, 0,  r };

		vertices[vi++] = Netcode::Float3{ h / 2.0f, 0, -r };
		vertices[vi++] = Netcode::Float3{ -h / 2.0f, 0, -r };

		return vi;
	}

	uint32_t BasicGeometry::GetSphereWireframeSize(uint32_t numSlices)
	{
		return numSlices * (numSlices - 1) * 4;
	}

	uint32_t BasicGeometry::CreateSphereWireframe(void * dstData, uint32_t stride, float radius, uint32_t numSlices, uint32_t positionDataOffset)
	{
		VertexDataIterator vertices(dstData, stride, positionDataOffset);

		const float fNumSlices = static_cast<float>(numSlices);
		constexpr static float PI2 = DirectX::XM_2PI;

		uint32_t vi = 0;
		const Netcode::Vector3 unitZ = Netcode::Float3(0.0f, 0.0f, 1.0f);

		for(uint32_t i = 0; i < numSlices; ++i) {
			for(uint32_t j = 0; j < numSlices - 1; ++j) {
				float yawArg = (static_cast<float>(i) / fNumSlices) * PI2;
				float yawArgNext = (static_cast<float>((i + 1) % numSlices) / fNumSlices) * PI2;

				float pitchArg = (static_cast<float>(j) / fNumSlices) * PI2;
				float pitchArgNext = (static_cast<float>(j + 1) / fNumSlices) * PI2;

				Netcode::Quaternion q0(pitchArg, yawArg, 0.0f);
				Netcode::Quaternion q1(pitchArgNext, yawArg, 0.0f);
				Netcode::Quaternion q2(pitchArgNext, yawArg, 0.0f);
				Netcode::Quaternion q3(pitchArgNext, yawArgNext, 0.0f);

				vertices[vi++] = unitZ.Rotate(q0) * radius;
				vertices[vi++] = unitZ.Rotate(q1) * radius;
				vertices[vi++] = unitZ.Rotate(q2) * radius;
				vertices[vi++] = unitZ.Rotate(q3) * radius;
			}
		}

		return vi;
	}
}
