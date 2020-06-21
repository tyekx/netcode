#pragma once

#include "Vector3.h"

namespace Netcode {

	class GramSchmidt {
		Netcode::Float3 b0;
		Netcode::Float3 b1;
		Netcode::Float3 b2;

		Netcode::Vector3 Projection(Netcode::Vector3 lhs, Netcode::Vector3 baseVec);

	public:
		inline const Netcode::Float3 & GetBaseVec0() const {
			return b0;
		}

		inline const Netcode::Float3 & GetBaseVec1() const {
			return b1;
		}

		inline const Netcode::Float3 & GetBaseVec2() const {
			return b2;
		}

		GramSchmidt(const Netcode::Float3 & e0, const Netcode::Float3 & e1, const Netcode::Float3 & e2);

	};

}
