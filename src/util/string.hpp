#pragma once

#include "pch.hpp"

namespace util {

template<typename T>
struct is_string : public std::disjunction<
		std::is_same<char *, typename std::decay_t<T>>,
		std::is_same<const char *, typename std::decay_t<T>>,
		std::is_same<std::string, typename std::decay_t<T>>> {};

template<typename T>
constexpr bool is_string_v = is_string<T>::value;

template<typename T>
concept string_type = is_string_v<T>;

template<typename T>
std::string to_string(const T &value);

std::string to_string(bool value);

std::string to_string(int32_t value);

std::string to_string(float value, bool trim_zeros = true, int32_t precision = 6);

std::string to_string(double value, bool trim_zeros = true, int32_t precision = 6);

std::vector<std::string> split(std::string_view str,
		std::string_view delimiter, bool skip_empty = false);

}

#include "string.inl"
