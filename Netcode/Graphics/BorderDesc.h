#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode {

	enum class BorderType : uint32_t {
		NONE = 0,
		SOLID = 1
	};
	
	struct BorderDesc {
		BorderType type;
		float borderWidth;
		float borderRadius;
		Float4 color;

		bool IsEmpty() const;

		bool operator==(const BorderDesc & rhs) const;
		bool operator!=(const BorderDesc & rhs) const;

		BorderDesc();
		BorderDesc(float width, float radius, const Float4 & color);
	};

}
