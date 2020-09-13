#pragma once

#include "../Macros.h"
#include <DirectXMath.h>
#include <cstdint>
#include "NonZero.hpp"

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

		static const Float2 Zero;
		static const Float2 One;
		static const Float2 UnitX;
		static const Float2 UnitY;
		static const Float2 Eps;
	};

	struct Float3 : public DirectX::XMFLOAT3 {
		using DirectX::XMFLOAT3::XMFLOAT3;

		static const Float3 Zero;
		static const Float3 One;
		static const Float3 UnitX;
		static const Float3 UnitY;
		static const Float3 UnitZ;
		static const Float3 Eps;
	};

	struct Float4 : public DirectX::XMFLOAT4 {
		using DirectX::XMFLOAT4::XMFLOAT4;

		static const Float4 Zero;
		static const Float4 One;
		static const Float4 UnitX;
		static const Float4 UnitY;
		static const Float4 UnitZ;
		static const Float4 UnitW;
		static const Float4 Eps;
	};

	struct Float4x4 : public DirectX::XMFLOAT4X4 {
		using DirectX::XMFLOAT4X4::XMFLOAT4X4;

		Float4x4(const DirectX::XMFLOAT4X4 & rhs) noexcept : DirectX::XMFLOAT4X4::XMFLOAT4X4(rhs) { }

		static const Float4x4 Zero;
		static const Float4x4 Identity;
	};

	using Int = int32_t;

	struct Int2 : public DirectX::XMINT2 {
		using DirectX::XMINT2::XMINT2;

		static const Int2 Zero;
		static const Int2 One;
		static const Int2 UnitX;
		static const Int2 UnitY;
	};

	struct Int3 : public DirectX::XMINT3 {
		using DirectX::XMINT3::XMINT3;

		static const Int3 Zero;
		static const Int3 One;
		static const Int3 UnitX;
		static const Int3 UnitY;
		static const Int3 UnitZ;
	};

	struct Int4 : public DirectX::XMINT4 {
		using DirectX::XMINT4::XMINT4;

		static const Int4 Zero;
		static const Int4 One;
		static const Int4 UnitX;
		static const Int4 UnitY;
		static const Int4 UnitZ;
		static const Int4 UnitW;
	};

	using UInt = uint32_t;

	struct UInt2 : public DirectX::XMUINT2 {
		using DirectX::XMUINT2::XMUINT2;

		static const UInt2 Zero;
		static const UInt2 One;
		static const UInt2 UnitX;
		static const UInt2 UnitY;
	};

	struct UInt3 : public DirectX::XMUINT3 {
		using DirectX::XMUINT3::XMUINT3;

		static const UInt3 Zero;
		static const UInt3 One;
		static const UInt3 UnitX;
		static const UInt3 UnitY;
		static const UInt3 UnitZ;
	};

	struct UInt4 : public DirectX::XMUINT4 {
		using DirectX::XMUINT4::XMUINT4;

		static const UInt4 Zero;
		static const UInt4 One;
		static const UInt4 UnitX;
		static const UInt4 UnitY;
		static const UInt4 UnitZ;
		static const UInt4 UnitW;
	};

}
