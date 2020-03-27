#pragma once

#include "../Common.h"
#include "ResourceEnums.h"

#include <stdint.h>

#if defined(EGG_OS_WINDOWS)
#include <dxgiformat.h>
#endif

namespace Egg::Graphics {

	struct ClearValue {
		DXGI_FORMAT Format;
		union
		{
			float Color[4];
			struct {
				float Depth;
				uint8_t Stencil;
			} DepthStencil;
		};
	};

	struct ResourceDesc {
		uint64_t sizeInBytes;
		uint64_t width;
		uint32_t height;
		uint32_t strideInBytes;
		uint16_t mipLevels;
		uint16_t depth;
		ResourceType type;
		ResourceFlags flags;
		ResourceState state;
		ResourceDimension dimension;
		DXGI_FORMAT format;
		ClearValue clearValue;
	};

}
