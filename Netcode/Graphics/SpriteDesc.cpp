#include "SpriteDesc.h"

namespace Netcode {

	bool SpriteDesc::IsEmpty() const {
		return type == BackgroundType::NONE;
	}

	bool SpriteDesc::operator==(const SpriteDesc & rhs) const {
		return rhs.type == type && texture == rhs.texture && textureSize.x == rhs.textureSize.y && textureSize.y == rhs.textureSize.y;
	}

	bool SpriteDesc::operator!=(const SpriteDesc & rhs) const {
		return !operator==(rhs);
	}

	SpriteDesc::SpriteDesc() :
		type{ BackgroundType::NONE },
		texture{ nullptr },
		textureSize{ UInt2::Zero },
		color{ Float4::Zero },
		sourceRect{ 0, 0, 0, 0 } {

	}

	SpriteDesc::SpriteDesc(const Netcode::Float4 & color) :
		type{ BackgroundType::SOLID },
		texture{ nullptr },
		textureSize{ UInt2::Zero },
		color{ color },
		sourceRect{ 0, 0, 0, 0 } {

	}

	SpriteDesc::SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize) :
		SpriteDesc(texture, textureSize, Float4::One) { }

	SpriteDesc::SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize, const Float4 & albedoColor) :
		SpriteDesc(texture, textureSize, Rect{ 0, 0, static_cast<int32_t>(textureSize.x), static_cast<int32_t>(textureSize.y) }, albedoColor) {

	}

	SpriteDesc::SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize, const Rect & sourceRect) :
		SpriteDesc(texture, textureSize, sourceRect, Float4::One) {

	}

	SpriteDesc::SpriteDesc(const Ref<ResourceViews> & texture, const UInt2 & textureSize, const Rect & sourceRect, const Float4 & albedoColor) :
		type{ BackgroundType::TEXTURE },
		texture{ texture },
		textureSize{ textureSize },
		color{ albedoColor },
		sourceRect{ sourceRect } {

	}

}
