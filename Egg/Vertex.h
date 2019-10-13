#pragma once

#include "Math/Float3.h"
#include "Math/UInt4.h"

namespace Egg {
	/*
	PNT: Position, Normal, Texture
	*/
	struct PNT_Vertex {
		constexpr static int type = 0;

		Egg::Math::Float3 position;
		Egg::Math::Float3 normal;
		Egg::Math::Float2 tex;
	};

	/*
	PNTTB: Position, Normal, Texture, Tangent, Binormal
	*/
	struct PNTTB_Vertex {
		constexpr static int type = 1;

		Egg::Math::Float3 position;
		Egg::Math::Float3 normal;
		Egg::Math::Float2 tex;
		Egg::Math::Float3 tangent;
		Egg::Math::Float3 binormal;
	};

	/*
	PNTWB: Position, Normal, Texture, Weights, BoneIds
	*/
	struct PNTWB_Vertex {
		constexpr static int type = 2;

		Egg::Math::Float3 position;
		Egg::Math::Float3 normal;
		Egg::Math::Float2 tex;
		Egg::Math::Float3 weights;
		Egg::Math::Int4 boneIds;
	};

	/*
	PNTTBWB: Position, Normal, Texture, Weights, BoneIds, Tangent, Binormal
	Animated normal mapped mesh
	*/
	struct PNTWBTB_Vertex {
		constexpr static int type = 3;

		Egg::Math::Float3 position;
		Egg::Math::Float3 normal;
		Egg::Math::Float2 tex;
		Egg::Math::Float3 weights;
		Egg::Math::Int4 boneIds;
		Egg::Math::Float3 tangent;
		Egg::Math::Float3 binormal;
	};

}
