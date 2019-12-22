#pragma once

namespace Egg {

	using HMATERIAL = unsigned int;
	using HITEM = unsigned int;
	using HSHADER = unsigned int;
	using HGEOMETRY = unsigned int;
	using HTEXTURE = unsigned int;
	using HCBUFFER = unsigned long long;
	using HRENDERTARGET = unsigned int;
	using HPSO = unsigned int;
	using HFONT = unsigned int;

	using HACTOR = void *;
	using HSHAPE = void *;
	using HPXMAT = void *;

	class HINCOMPLETESHADER {
	public:
		unsigned int handle;
	};

	enum class EGeometryType : unsigned {
		VERTEX_STREAM, INDEXED
	};

	static constexpr HSHADER SHADER_NOT_FOUND = 0xFFFFFFFF;

}
