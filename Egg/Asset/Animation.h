#pragma once

#include "../Math/Math.h"

namespace Egg {

	namespace Asset {

		enum class AnimationState {
			DEFAULT = 0,
			CONSTANT = 1,
			LINEAR = 2,
			REPEAT = 3
		};

		struct AnimationKey {
			Egg::Math::Float3 position;
			Egg::Math::Float4 rotation;
			Egg::Math::Float3 scale;
			double time;
		};

		struct BoneAnimation {
			int boneId;

			AnimationState preState;
			AnimationState postState;

			unsigned int keysLength;
			AnimationKey * keys;
		};

		struct Animation {
			char name[56];

			double duration;
			double ticksPerSecond;

			unsigned int boneDataLength;
			BoneAnimation * boneData;
		};

	}

}
