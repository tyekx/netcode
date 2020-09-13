#pragma once

#include <type_traits>
#include <cstdint>
namespace Netcode {


	template<typename T>
	class Enum {
	public:
		using Literals = T;
		using UnderlyingType = typename std::underlying_type<T>::type;
		static_assert(std::is_enum<T>::value, "T must be an enum or enum class");

	private:
		T value;

	public:
		~Enum() = default;
		constexpr Enum() noexcept : value{ T{} } { }
		constexpr Enum(const Enum<T> & rhs) noexcept : value{ rhs.value } { }
		constexpr Enum(const T & rhs) noexcept : value{ rhs } { }
		constexpr Enum(Enum<T> && rhs) noexcept : value{ rhs.value } { }
		
		constexpr Enum<T> & operator=(const Enum<T> & rhs) noexcept {
			value = rhs.value;
			return *this;
		}
		
		constexpr Enum<T> & operator=(Enum<T> && rhs) noexcept {
			value = rhs.value;
			return *this;
		}

		constexpr Enum<T> & operator|=(const Enum<T> & rhs) noexcept {
			*this = *this | rhs;
			return *this;
		}

		constexpr Enum<T> & operator&=(const Enum<T> & rhs) noexcept {
			*this = *this & rhs;
			return *this;
		}
		
		constexpr Enum<T> operator&(const Enum<T> & rhs) const noexcept {
			return static_cast<T>(static_cast<UnderlyingType>(value) & static_cast<UnderlyingType>(rhs.value));
		}

		constexpr Enum<T> operator|(const Enum<T> & rhs) const noexcept {
			return static_cast<T>(static_cast<UnderlyingType>(value) | static_cast<UnderlyingType>(rhs.value));
		}

		constexpr bool operator==(const Enum<T> & rhs) const noexcept {
			return value == rhs.value;
		}

		constexpr Enum<T> operator~() const noexcept {
			return ~value;
		}

		constexpr explicit operator T() const noexcept {
			return value;
		}

		[[nodiscard]]
		constexpr bool IsSet(const Enum<T> & rhs) const noexcept {
			return static_cast<T>(*this & rhs) == rhs.value;
		}

		[[nodiscard]]
		constexpr bool IsAnySet(const Enum<T> & rhs) const noexcept {
			return static_cast<UnderlyingType>((*this & rhs).value) > static_cast<UnderlyingType>(0);
		}

		constexpr void Set(const Enum<T> & rhs) noexcept {
			*this |= rhs;
		}
		
		constexpr void Unset(const Enum<T> & rhs) noexcept {
			*this &= ~rhs;
		}
	};
	
}

#if !defined(NETCODE_ENUM_CLASS_OPERATORS)

#define NETCODE_ENUM_CLASS_OPERATORS(type)																	\
constexpr Netcode::Enum< type > operator&(type lhs, type rhs) {												\
	return Netcode::Enum< type >(lhs) & rhs;																\
}																											\
constexpr Netcode::Enum< type > operator|(type lhs, type rhs) {												\
	return Netcode::Enum< type >(lhs) | rhs;																\
}																											\
constexpr Netcode::Enum< type > operator~(type lhs) {														\
	return ~Netcode::Enum< type >(lhs);																		\
}

#endif
