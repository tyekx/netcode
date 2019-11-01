#pragma once

namespace Egg {


	class MovementController {
	public:
		bool IsMovingForward() const {
			return false;
		}

		bool IsMovingLeft() const {
			return false;
		}

		bool IsMovingRight() const {
			return false;
		}

		bool IsMovingBackward() const {
			return IsOnGround(); // and only AxisY is active
		}

		bool IsMovingForwardLeft() const {
			return false;
		}

		bool IsMovingForwardRight() const {
			return false;
		}

		bool IsMovingBackwardLeft() const {
			return false;
		}

		bool IsMovingBackwardRight() const {
			return false;
		}

		bool IsJumping() const {
			return false;
		}

		bool IsFiring() const {
			return false;
		}

		bool IsOnGround() const {
			return false;
		}

		bool IsIdle() const {
			return true;
		}
	};

}
