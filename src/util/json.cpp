#include "json.hpp"

using namespace rapidjson;

namespace util {

static json parse_rapidjson_value(Value::ConstValueIterator value);

static std::string escape_json(std::string_view str);

json::json(number value) noexcept : value(value) {}

json::json(string &&value) noexcept : value(std::move(value)) {}

json::json(const array &value) : value(value) {}

json::json(array &&value) noexcept : value(std::move(value)) {}

json::json(const object &value) : value(value) {}

json::json(object &&value) : value(std::move(value)) {}

json::operator number() const {
	return as<number>();
}

json::operator number &() {
	return as<number>();
}

json::operator const string &() const {
	return as<string>();
}

json::operator string &() {
	return as<string>();
}

json::operator const array &() const {
	return as<array>();
}

json::operator array &() {
	return as<array>();
}

json::operator const object &() const {
	return as<object>();
}

json::operator object &() {
	return as<object>();
}

std::ostream &operator<<(std::ostream &lhs, const json &rhs) {
	return lhs << rhs.to_string();
}

json &json::operator[](size_t index) {
	return as<array>()[index];
}

const json &json::operator[](size_t index) const {
	return as<array>()[index];
}

json json::parse(std::string_view json) {
	Document document;

	ParseResult result = document.Parse(json.data());
	if (!result) {
		throw error(GetParseError_En(result.Code())
				+ std::string(" (")
				+ std::to_string(result.Offset())
				+ ')');
	}

	return parse_rapidjson_value(&document);
}

std::string json::to_string(uint8_t current_indent) const {
	if (is<json::null>())
		return "null";

	if (is<json::boolean>())
		return util::to_string(as<json::boolean>());

	if (is<json::number>())
		return util::to_string(as<json::number>());

	if (is<json::string>())
		return '"' + escape_json(as<json::string>()) + '"';

	std::string indent_str(current_indent, '\t');
	std::ostringstream ss;

	if (is<json::array>()) {
		const json::array array = as<json::array>();

		ss << '[';
		for (auto it = array.cbegin(); it != array.cend(); it++) {
			ss << "\n\t" << indent_str
					<< it->to_string(current_indent + 1)
					<< (std::next(it) == array.cend() ? '\n' : ',');
		}

		// If no newlines occured
		if (array.size() != 0)
			ss << indent_str;

		ss << ']';
	} else if (is<json::object>()) {
		const json::object object = as<json::object>();

		ss << '{';
		for (auto it = object.cbegin(); it != object.cend(); it++) {
			ss << "\n\t" << indent_str
					<< '"' << escape_json(it->first) << "\": " << it->second.to_string(current_indent + 1)
					<< (std::next(it) == object.cend() ? '\n' : ',');
		}

		// If no newlines occured
		if (object.size() != 0)
			ss << indent_str;
			
		ss << '}';
	}

	return ss.str();
}

static json parse_rapidjson_value(Value::ConstValueIterator value) {
	if (value->IsBool())
		return value->GetBool();

	if (value->IsNumber())
		return value->GetDouble();
	
	if (value->IsString())
		return std::string(value->GetString());
	
	if (value->IsArray()) {
		json json = json::array();

		Value::ConstArray json_array = value->GetArray();
		json::array &array = json.as<json::array>();
		array.reserve(json_array.Size());

		for (Value::ConstValueIterator it = json_array.Begin(); it != json_array.End(); it++)
			array.push_back(parse_rapidjson_value(it));

		return json;
	}

	if (value->IsObject()) {
		json json = json::object();

		Value::ConstObject json_object = value->GetObject();
		json::object &object = json.as<json::object>();
		// object.reserve(json_object.MemberCount());

		for (Value::ConstMemberIterator it = json_object.MemberBegin(); it != json_object.MemberEnd(); it++)
			object.emplace(it->name.GetString(), parse_rapidjson_value(&it->value));

		return json;
	}

	return json();
}

static std::string escape_json(std::string_view str) {
    std::ostringstream ss;

    for (char c : str) {
        switch (c) {
        case  '"': ss << "\\\""; break;
        case '\\': ss << "\\\\"; break;
        case '\b': ss << "\\b";  break;
        case '\f': ss << "\\f";  break;
        case '\n': ss << "\\n";  break;
        case '\r': ss << "\\r";  break;
        case '\t': ss << "\\t";  break;
        default:
            if ('\x00' <= c && c <= '\x1f') {
                ss << "\\u" << std::hex << std::setw(4)
						<< std::setfill('0') << static_cast<int32_t>(c);
            } else {
                ss << c;
            }
        }
    }

    return ss.str();
}

}
