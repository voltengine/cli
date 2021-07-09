namespace util {

template<std::same_as<json::boolean> Boolean>
json::json(Boolean value) noexcept : value(value) {}

template<string_type String>
json::json(const String &value) : value(static_cast<string>(value)) {}

template<std::same_as<json::boolean> Boolean>
json::operator Boolean() const {
	return as<boolean>();
}

template<std::same_as<json::boolean> Boolean>
json::operator Boolean &() {
	return as<boolean>();
}

template<string_type Key>
json &json::operator[](const Key &key) {
	return as<object>()[key];
}

template<string_type Key>
const json &json::operator[](const Key &key) const {
	return as<object>().at(key);
}

template<json_type T>
bool json::is() const noexcept {
	return std::holds_alternative<T>(value);
}

template<json_type T>
T &json::as() {
	if (!is<T>())
		throw std::runtime_error("Bad JSON type.");
	return std::get<T>(value);
}

template<json_type T>
const T &json::as() const {
	if (!is<T>())
		throw std::runtime_error("Bad JSON type.");
	return std::get<T>(value);
}

}
