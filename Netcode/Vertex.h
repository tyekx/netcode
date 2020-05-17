#pragma once

#include <DirectXMath.h>

namespace Netcode {
	/*
	PNT: Position, Normal, Texture
	*/
	struct PNT_Vertex {
		constexpr static int type = 0;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 tex;
	};

	/*
	PNTTB: Position, Normal, Texture, Tangent, Binormal
	*/
	struct PNTTB_Vertex {
		constexpr static int type = 1;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
	};

	/*
	PNTWB: Position, Normal, Texture, Weights, BoneIds
	*/
	struct PNTWB_Vertex {
		constexpr static int type = 2;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT3 weights;
		uint32_t boneIds; // 4 bytes, each byte is an id, 0xFF means no bone was referred
	};

	/*
	PNTTBWB: Position, Normal, Texture, Weights, BoneIds, Tangent, Binormal
	Animated normal mapped mesh
	*/
	struct PNTWBTB_Vertex {
		constexpr static int type = 3;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT3 weights;
		uint32_t boneIds; // 4 bytes, each byte is an id, 0xFF means no bone was referred
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
	};

	struct PCT_Vertex {
		constexpr static int type = 4;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 textureCoordinate;
	};

	struct PC_Vertex {
		constexpr static int type = 5;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
	};

}
