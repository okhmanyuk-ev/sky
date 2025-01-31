#pragma once

#include <type_traits>

namespace sky
{
	template <typename T>
	struct enable_bitmask_operators : std::false_type
	{
	};

	template <typename E>
	concept BitmaskEnum = std::is_enum_v<E> && enable_bitmask_operators<E>::value;

	template <typename E>
	class BitFlags
	{
		static_assert(std::is_enum_v<E>, "E must be an enum type");

	public:
		BitFlags(std::underlying_type_t<E> value) : mValue(value)
		{
		}

		BitFlags(E value) : mValue(static_cast<std::underlying_type_t<E>>(value))
		{
		}

		constexpr operator std::underlying_type_t<E>() const noexcept
		{
			return mValue;
		}

		constexpr BitFlags operator|=(BitFlags value) noexcept
		{
			mValue |= value;
			return *this;
		}

	private:
		std::underlying_type_t<E> mValue;
	};
}

template <sky::BitmaskEnum E>
constexpr std::underlying_type_t<E> operator|(std::underlying_type_t<E> a, E b) noexcept
{
	return a | static_cast<std::underlying_type_t<E>>(b);
}

template <sky::BitmaskEnum E>
constexpr std::underlying_type_t<E> operator|(E a, E b) noexcept
{
	return static_cast<std::underlying_type_t<E>>(a) | b;
}

template <sky::BitmaskEnum E>
constexpr std::underlying_type_t<E> operator&(std::underlying_type_t<E> a, E b) noexcept
{
	return a & static_cast<std::underlying_type_t<E>>(b);
}

template <sky::BitmaskEnum E>
constexpr std::underlying_type_t<E> operator~(E a) noexcept
{
	return ~static_cast<std::underlying_type_t<E>>(a);
}

template <sky::BitmaskEnum E>
constexpr std::underlying_type_t<E> operator|=(std::underlying_type_t<E> a, E b) noexcept
{
	a = a | b;
	return a;
}

template <sky::BitmaskEnum E, typename T>
	requires std::is_integral_v<T>
constexpr std::underlying_type_t<E> operator<<(E a, T n) noexcept
{
	return static_cast<T>(static_cast<std::underlying_type_t<E>>(a) << n);
}

template <sky::BitmaskEnum E>
constexpr bool operator==(E a, std::underlying_type_t<E> b) noexcept
{
	return static_cast<std::underlying_type_t<E>>(a) == b;
}

template <sky::BitmaskEnum E>
constexpr bool operator<(std::underlying_type_t<E> a, E b) noexcept
{
	return a < static_cast<std::underlying_type_t<E>>(b);
}
