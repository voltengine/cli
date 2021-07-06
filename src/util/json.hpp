#pragma once

#include "pch.hpp"

#include "util/string.hpp"

namespace util {

class json;

template<typename T>
concept json_type =
		std::is_same_v<T, nullptr_t>    ||
		std::is_same_v<T, bool> ||
		std::is_same_v<T, double>  ||
		std::is_same_v<T, std::string>  ||
		std::is_same_v<T, std::vector<json>>   ||
		std::is_same_v<T, std::map<std::string, json>>;

class json {
public:
	using null = nullptr_t;
	using boolean = bool;
	using number = double;
	using string = std::string;
	using array = std::vector<json>;
	using object = std::map<std::string, json>;

	using error = std::runtime_error;

	json() noexcept = default;

	json(null) noexcept {}

	template<std::same_as<json::boolean> Boolean>
	json(Boolean value) noexcept;

	json(number value) noexcept;

	template<string_type String>
	json(const String &value);

	json(string &&value) noexcept;

	json(const array &value);

	json(array &&value) noexcept;

	json(const object &value);

	json(object &&value);

	template<std::same_as<json::boolean> Boolean>
	operator Boolean() const;

	template<std::same_as<json::boolean> Boolean>
	operator Boolean &();

	operator number() const;

	operator number &();

	template<string_type String>
	operator const String &() const;

	template<string_type String>
	operator String &();

	operator const array &() const;

	operator array &();

	operator const object &() const;

	operator object &();

	friend std::ostream &operator<<(std::ostream &lhs, const json &rhs);

	json &operator[](size_t index);

	const json &operator[](size_t index) const;

	template<string_type Key>
	json &operator[](const Key &key);

	template<string_type Key>
	const json &operator[](const Key &key) const;

	bool operator==(const json &json) const = default;

	static json parse(std::string_view json);

	template<json_type T>
	inline bool is() const noexcept;

	template<json_type T>
	inline T &as();

	template<json_type T>
	inline const T &as() const;

private:
	std::variant<null, boolean, number, string, array, object> value;

	std::string to_string(uint8_t current_indent = 0) const;
};

}

#include "json.inl"
