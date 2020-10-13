#include "BorderDesc.h"

namespace Netcode {

	bool BorderDesc::IsEmpty() const {
		return type == BorderType::NONE;
	}

	bool BorderDesc::operator==(const BorderDesc & rhs) const {
		return type == rhs.type && borderWidth == rhs.borderWidth && borderRadius == rhs.borderRadius;
	}

	bool BorderDesc::operator!=(const BorderDesc & rhs) const {
		return !operator==(rhs);
	}

	BorderDesc::BorderDesc() : color{ Float4::Zero } , type{ BorderType::NONE }, borderWidth{ 0.0f }, borderRadius{ 0.0f } { }

	BorderDesc::BorderDesc(float width, float radius, const Float4 & color) :
		color{ color },
		type{ BorderType::SOLID },
		borderWidth{ width },
		borderRadius{ radius } { }

}
