#pragma once

#include "../Math/Float4x4.h" 

namespace Egg {

	namespace Asset {

		struct Bone {

			char name[28];
			int parentId;
			Egg::Math::Float4x4 transform;

		};

	}

}
