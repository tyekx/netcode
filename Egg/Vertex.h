#pragma once

#include <DirectXMath.h>

namespace Egg {
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
		DirectX::XMINT4 boneIds;
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
		DirectX::XMINT4 boneIds;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 binormal;
	};

}
