#pragma once

#include "Animation.h"


namespace Egg {

	class AnimationController {


	public:

		AnimationController(Animation * animations, unsigned int animationsLength) {
			
		}

		/*
		* Will write dstTransforms, size expected to match the constant buffer type's max bone count
		*/
		void Animate(DirectX::XMFLOAT4X4A * dstTransforms, float dt) {


		}

	};

}
