#pragma once

#include "pch.hpp"

#include "util/string.hpp"

namespace util {

class json;

template<typename T>
concept json_type =
		std::is_same_v<T, std::nullptr_t>    ||
		std::is_same_v<T, bool> ||
		std::is_same_v<T, double>  ||
		std::is_same_v<T, std::string>  ||
		std::is_same_v<T, std::vector<json>>   ||
		std::is_same_v<T, std::unordered_map<std::string, json>>;

class json {
public:
	using null = std::nullptr_t;
	using boolean = bool;
	using number = double;
	using string = std::string;
	using array = std::vector<json>;
	using object = std::unordered_map<std::string, json>;

	json() = default;

	json(null);

	template<std::same_as<json::boolean> Boolean>
	json(Boolean value);

	json(number value);

	json(const string &value);

	json(string &&value);

	json(const array &value);

	json(array &&value);

	json(const object &value);

	json(object &&value);

	template<std::same_as<json::boolean> Boolean>
	operator Boolean() const;

	template<std::same_as<json::boolean> Boolean>
	operator Boolean &();

	operator number() const;

	operator number &();

	operator const string &() const;

	operator string &();

	operator const array &() const;

	operator array &();

	operator const object &() const;

	operator object &();

	json &operator[](size_t index);

	const json &operator[](size_t index) const;

	template<string_type Key>
	json &operator[](const Key &key);

	template<string_type Key>
	const json &operator[](const Key &key) const;

	bool operator==(const json &json) const = default;

	static json from_string(const std::string &json);

	template<json_type T>
	bool is() const;

	template<json_type T>
	T &as();

	template<json_type T>
	const T &as() const;

	std::string to_string(uint8_t indent = 0) const;

private:

	std::variant<null, boolean, number, string, array, object> value;
};

std::ostream &operator<<(std::ostream &lhs, const json &rhs);

}

#include "json.inl"
