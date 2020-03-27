#pragma once

#include "Input.h"

namespace Netcode {


	class MovementController {
		float dirX;
		float dirY;
		float dirZ;
		float firing;

	public:
		void Update() {
			dirX = Input::GetAxis("Horizontal");
			dirY = Input::GetAxis("Vertical");
			dirZ = Input::GetAxis("Jump");
			firing = Input::GetAxis("Fire");
		}

		bool IsMovingForward() const {
			return dirY > 0.0f && dirX == 0.0f && IsOnGround();
		}

		bool IsMovingLeft() const {
			return dirY == 0.0f && dirX > 0.0f && IsOnGround();
		}

		bool IsMovingRight() const {
			return dirY == 0.0f && dirX < 0.0f && IsOnGround();
		}

		bool IsMovingBackward() const {
			return dirY < 0.0f && dirX == 0.0f && IsOnGround();
		}

		bool IsMovingForwardLeft() const {
			return dirY > 0.0f && dirX > 0.0f && IsOnGround();
		}

		bool IsMovingForwardRight() const {
			return dirY > 0.0f && dirX < 0.0f && IsOnGround();
		}

		bool IsMovingBackwardLeft() const {
			return dirY < 0.0f && dirX > 0.0f && IsOnGround();
		}

		bool IsMovingBackwardRight() const {
			return dirY < 0.0f && dirX < 0.0f && IsOnGround();
		}

		bool IsJumping() const {
			return dirZ > 0.0f;
		}

		bool IsFiring() const {
			return firing > 0;
		}

		bool IsOnGround() const {
			return dirZ == 0.0f;
		}

		bool IsIdle() const {
			return dirX == 0.0f && dirY == 0.0f && IsOnGround();
		}
	};

}
