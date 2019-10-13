#pragma once

#include "EggMpl.hpp"

namespace Egg {

	namespace Signature {
		
		template<typename SYSTEM_T>
		bool IsCompatible(SignatureType signature) {
			return ((signature & SYSTEM_T::Required()) > 0) &&
				   ((signature & SYSTEM_T::Incompatible()) == 0);
		}

	}

}
