#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode {
	/*
	PNT: Position, Normal, Texture
	*/
	struct PNT_Vertex {
		constexpr static int type = 0;

		Netcode::Float3 position;
		Netcode::Float3 normal;
		Netcode::Float2 tex;
	};

	/*
	PNTTB: Position, Normal, Texture, Tangent, Binormal
	*/
	struct PNTTB_Vertex {
		constexpr static int type = 1;

		Netcode::Float3 position;
		Netcode::Float3 normal;
		Netcode::Float2 tex;
		Netcode::Float3 tangent;
		Netcode::Float3 binormal;
	};

	/*
	PNTWB: Position, Normal, Texture, Weights, BoneIds
	*/
	struct PNTWB_Vertex {
		constexpr static int type = 2;

		Netcode::Float3 position;
		Netcode::Float3 normal;
		Netcode::Float2 tex;
		Netcode::Float3 weights;
		uint32_t boneIds; // 4 bytes, each byte is an id, 0xFF means no bone was referred
	};

	/*
	PNTTBWB: Position, Normal, Texture, Weights, BoneIds, Tangent, Binormal
	Animated normal mapped mesh
	*/
	struct PNTWBTB_Vertex {
		constexpr static int type = 3;

		Netcode::Float3 position;
		Netcode::Float3 normal;
		Netcode::Float2 tex;
		Netcode::Float3 weights;
		uint32_t boneIds; // 4 bytes, each byte is an id, 0xFF means no bone was referred
		Netcode::Float3 tangent;
		Netcode::Float3 binormal;
	};

	struct PCT_Vertex {
		constexpr static int type = 4;

		Netcode::Float3 position;
		Netcode::Float4 color;
		Netcode::Float2 textureCoordinate;
	};

	struct PC_Vertex {
		constexpr static int type = 5;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
	};

}
