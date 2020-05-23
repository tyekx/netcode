#include "NonZero.hpp"

namespace Netcode::Detail {

	void NonZeroAssertion(bool tf) {
#if defined(NETCODE_USE_MATH_EXCEPTIONS)
		if(!tf) {
			throw Netcode::ArithmeticException{ "Expression evaluated to zero" };
		}
#else
		assert(tf);
#endif
	}

	void LengthAssertion(bool tf)
	{
#if defined(NETCODE_USE_MATH_EXCEPTIONS)
		if(!tf) {
			throw Netcode::ArithmeticException{ "Expression is not normalized" };
		}
#else
		assert(tf);
#endif
	}

}
