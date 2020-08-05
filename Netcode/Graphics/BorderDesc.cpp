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

	BorderDesc::BorderDesc() : type{ BorderType::NONE }, borderWidth{ 0.0f }, borderRadius{ 0.0f }, color{ Float4::Zero } { }

	BorderDesc::BorderDesc(float width, float radius, const Float4 & color) :
		type{ BorderType::SOLID },
		borderWidth{ width },
		borderRadius{ radius },
		color{ color } { }

}
