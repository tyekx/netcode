#include "GramSchmidt.h"

namespace Netcode {



	Netcode::Vector3 GramSchmidt::Projection(Netcode::Vector3 lhs, Netcode::Vector3 baseVec) {
		return (lhs.Dot(baseVec) / baseVec.Dot(baseVec)) * baseVec;
	}

	GramSchmidt::GramSchmidt(const Netcode::Float3 & e0, const Netcode::Float3 & e1, const Netcode::Float3 & e2) {
		Netcode::Vector3 v0 = e0;
		Netcode::Vector3 v1 = e1;
		Netcode::Vector3 v2 = e2;

		Netcode::Vector3 u0 = v0;
		Netcode::Vector3 u1 = v1 - Projection(v1, u0);
		Netcode::Vector3 u2 = v2 - Projection(v2, u0) - Projection(v2, u1);

		b0 = u0.Normalize();
		b1 = u1.Normalize();
		b2 = u2.Normalize();
	}

}
