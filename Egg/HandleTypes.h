#pragma once

#include "Common.h"

#if defined(EGG_OS_WINDOWS)
#include <DirectXTex/DirectXTex.h>
#endif

namespace Egg {

	using HGEOMETRY = unsigned int;
	using HSHADER = unsigned int;
	using HTEXTURE = unsigned int;
	using HPSO = unsigned int;
	using HFONT = unsigned int;

	using HACTOR = void *;
	using HSHAPE = void *;
	using HPXMAT = void *;

	static constexpr HSHADER SHADER_NOT_FOUND = 0xFFFFFFFF;

#if !defined(EGG_OS_WINDOWS)

	struct Image
	{
		size_t      width;
		size_t      height;
		DXGI_FORMAT format;
		size_t      rowPitch;
		size_t      slicePitch;
		uint8_t * pixels;
	};

	struct RECT {
		int left;
		int top;
		int right;
		int bottom;
	};

#else

	using Image = DirectX::Image;

#endif


}
