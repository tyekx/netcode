#pragma once

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

	constexpr float C_PI = DirectX::XM_PI;
	constexpr float C_2PI = DirectX::XM_2PI;
	constexpr float C_1DIVPI = DirectX::XM_1DIVPI;
	constexpr float C_1DIV2PI = DirectX::XM_1DIV2PI;
	constexpr float C_PIDIV2 = DirectX::XM_PIDIV2;
	constexpr float C_PIDIV4 = DirectX::XM_PIDIV4;
	constexpr float C_Eps = 1.192092896e-7f;
	constexpr float C_Euler = 2.718281746f;

	using Float = float;

	struct Float2 : public DirectX::XMFLOAT2 {
		using DirectX::XMFLOAT2::XMFLOAT2;

		static Float2 Zero;
		static Float2 One;
		static Float2 UnitX;
		static Float2 UnitY;
		static Float2 Eps;
	};

	struct Float3 : public DirectX::XMFLOAT3 {
		using DirectX::XMFLOAT3::XMFLOAT3;

		static Float3 Zero;
		static Float3 One;
		static Float3 UnitX;
		static Float3 UnitY;
		static Float3 UnitZ;
		static Float3 Eps;
	};

	struct Float4 : public DirectX::XMFLOAT4 {
		using DirectX::XMFLOAT4::XMFLOAT4;

		static Float4 Zero;
		static Float4 One;
		static Float4 UnitX;
		static Float4 UnitY;
		static Float4 UnitZ;
		static Float4 UnitW;
		static Float4 Eps;
	};

	struct Float4x4 : public DirectX::XMFLOAT4X4 {
		using DirectX::XMFLOAT4X4::XMFLOAT4X4;

		Float4x4(const DirectX::XMFLOAT4X4 & rhs) noexcept : DirectX::XMFLOAT4X4::XMFLOAT4X4(rhs) { }

		static Float4x4 Zero;
		static Float4x4 Identity;
	};

	using Int = int32_t;

	struct Int2 : public DirectX::XMINT2 {
		using DirectX::XMINT2::XMINT2;

		static Int2 Zero;
		static Int2 One;
		static Int2 UnitX;
		static Int2 UnitY;
	};

	struct Int3 : public DirectX::XMINT3 {
		using DirectX::XMINT3::XMINT3;

		static Int3 Zero;
		static Int3 One;
		static Int3 UnitX;
		static Int3 UnitY;
		static Int3 UnitZ;
	};

	struct Int4 : public DirectX::XMINT4 {
		using DirectX::XMINT4::XMINT4;

		static Int4 Zero;
		static Int4 One;
		static Int4 UnitX;
		static Int4 UnitY;
		static Int4 UnitZ;
		static Int4 UnitW;
	};

	using UInt = uint32_t;

	struct UInt2 : public DirectX::XMUINT2 {
		using DirectX::XMUINT2::XMUINT2;

		static UInt2 Zero;
		static UInt2 One;
		static UInt2 UnitX;
		static UInt2 UnitY;
	};

	struct UInt3 : public DirectX::XMUINT3 {
		using DirectX::XMUINT3::XMUINT3;

		static UInt3 Zero;
		static UInt3 One;
		static UInt3 UnitX;
		static UInt3 UnitY;
		static UInt3 UnitZ;
	};

	struct UInt4 : public DirectX::XMUINT4 {
		using DirectX::XMUINT4::XMUINT4;

		static UInt4 Zero;
		static UInt4 One;
		static UInt4 UnitX;
		static UInt4 UnitY;
		static UInt4 UnitZ;
		static UInt4 UnitW;
	};

}
