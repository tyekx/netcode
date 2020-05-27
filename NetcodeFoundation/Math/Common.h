#include <DirectXMath.h>
#include <cstdint>
#include "NonZero.hpp"

#define DEFAULT_NOEXCEPT_ALL6(type) type() noexcept = default; \
~type() noexcept = default; \
type(const type & rhs) noexcept = default; \
type(type &&) noexcept = default; \
type & operator=(const type & rhs) noexcept = default; \
type & operator=(type && rhs) noexcept = default

#if !defined(XM_CALLCONV)
#define NC_MATH_CALLCONV __stdcall
#else
#define NC_MATH_CALLCONV XM_CALLCONV
#endif

#if defined(NETCODE_USE_MATH_EXCEPTIONS)
#define NETCODE_MATH_DEP_NOEXCEPT
#else
// dependent noexcept
#define NETCODE_MATH_DEP_NOEXCEPT noexcept
#endif

namespace Netcode {

	using Float = float;
	using Float2 = DirectX::XMFLOAT2;
	using Float3 = DirectX::XMFLOAT3;
	using Float4 = DirectX::XMFLOAT4;
	using Float4x4 = DirectX::XMFLOAT4X4;

	using Int = int32_t;
	using Int2 = DirectX::XMINT2;
	using Int3 = DirectX::XMINT3;
	using Int4 = DirectX::XMINT4;

	using UInt = uint32_t;
	using UInt2 = DirectX::XMUINT2;
	using UInt3 = DirectX::XMUINT3;
	using UInt4 = DirectX::XMUINT4;

}
