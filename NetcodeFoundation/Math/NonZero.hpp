#pragma once

#include <cassert>
#if defined(NETCODE_USE_MATH_EXCEPTIONS)
#include "../Exceptions.h"
#endif

namespace Netcode {

	namespace Detail {

		void NonZeroAssertion(bool tf);
		void LengthAssertion(bool tf);

	}


	/**
	 * @class NonZero<T>
	 * checks if any component is Zero for the given type, check is done with == 0.0f.
	 * The goal is to catch (nand) values right when they are about to be created
	 * type must meet the ZeroCheckConcept
	 * ZeroCheckConcept: must have bool AnyZero() const, bool AllZero() const member functions
	 * must be copy constructible
	 */
	template<typename ZeroCheckConcept>
	class NonZero {
		const ZeroCheckConcept & v;

	public:
		NonZero(const ZeroCheckConcept & rhs) : v{ rhs } {
			Detail::NonZeroAssertion(!rhs.AnyZero());
		}

		operator ZeroCheckConcept() const {
			return v;
		}
	};
	
	template<>
	class NonZero<float> {
		float v;
	public:
		NonZero(float rhs) : v{ rhs } {
			Detail::NonZeroAssertion(rhs != 0.0f);
		}

		operator float() const {
			return v;
		}
	};


	/**
	* @class NonZeroMagnitude<T>
	* checks if all components are Zero for the given type
	* For type float use NonZero<float>
	*/
	template<typename ZeroCheckConcept>
	class NotNullVector {
		ZeroCheckConcept v;
	public:
		NotNullVector(const ZeroCheckConcept & rhs) : v{ rhs } {
			Detail::NonZeroAssertion(!rhs.AllZero());
		}

		operator ZeroCheckConcept() const {
			return v;
		}
	};

	/**
	LengthCheckConcept: Has float LengthSq() const member function
	*/
	template<typename LengthCheckConcept>
	class Normalized {
		LengthCheckConcept v;
	public:
		Normalized(const LengthCheckConcept & n) : v{ n } {
			constexpr float eps = 0.001f;
			Detail::LengthAssertion((1.0f - eps) < n.LengthSq() && n.LengthSq() < (1.0f + eps));
		}

		operator LengthCheckConcept() const {
			return v;
		}
	};

}
