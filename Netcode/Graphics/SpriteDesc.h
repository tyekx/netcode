#pragma once

#include <NetcodeFoundation/Math.h>
#include <Netcode/HandleDecl.h>
#include <memory>

namespace Netcode {

	enum class BackgroundType : unsigned {
		NONE = 0,
		SOLID = 1,
		TEXTURE = 2
	};

	struct SpriteDesc {
		BackgroundType type;
		Ref<ResourceViews> texture;
		UInt2 textureSize;
		Float4 color;
		Rect sourceRect;

		bool IsEmpty() const;
		bool operator==(const SpriteDesc & rhs) const;
		bool operator!=(const SpriteDesc & rhs) const;

		SpriteDesc();
		SpriteDesc(const Float4 & color);
		SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize);
		SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize, const Float4 & albedoColor);
		SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize, const Rect & sourceRect);
		SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize, const Rect & sourceRect, const Float4 & albedoColor);
	};

}
