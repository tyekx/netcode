#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode::Asset {

	struct Bone {

		char name[64];
		int parentId;
		Netcode::Float4x4 transform;

	};

}
