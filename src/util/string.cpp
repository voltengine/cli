#include "string.hpp"

namespace util {

template<typename T>
static std::string format_fp(T value, bool trim_zeros, int32_t precision) {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(precision) << value;
	
	std::string str = ss.str();
	std::string trimmed = str;
	trimmed.erase(trimmed.find_last_not_of('0') + 1);

	if (trim_zeros) {
		str = trimmed;

		if (str == "-0.")
			str = "0";
		else if (str[str.size() - 1] == '.')
			str.pop_back();
	} else if (trimmed == "-0.")
		str.erase(str.begin());

	return str;
}

std::string to_string(bool value) {
	return value ? "true" : "false";
}

std::string to_string(int32_t value) {
	return std::to_string(value);
}

std::string to_string(float value, bool trim_zeros, int32_t precision) {
	return format_fp(value, trim_zeros, precision);
}

std::string to_string(double value, bool trim_zeros, int32_t precision) {
	return format_fp(value, trim_zeros, precision);
}

void ltrim(std::string &str) {
	auto end = std::find_if(str.begin(), str.end(),
			[](char c) { return !std::isspace(c); });
	str.erase(str.begin(), end);
}

void rtrim(std::string &str) {
	auto begin = std::find_if(str.rbegin(), str.rend(),
			[](char c) { return !std::isspace(c); }).base();
    str.erase(begin, str.end());
}

void trim(std::string &str) {
	ltrim(str);
	rtrim(str);
}

std::vector<std::string> split(std::string_view str,
		std::string_view delimiter, bool skip_empty) {
	std::vector<std::string> tokens;
	size_t pos = 0, len = str.size();

	while (pos <= len) {
		size_t new_pos = std::min(str.find(delimiter, pos), len);
		if (!skip_empty || new_pos != pos)
			tokens.push_back(std::string(str.substr(pos, new_pos - pos)));
		pos = new_pos + 1;
	}

	return tokens;
}

void replace(std::string &str, std::string_view from, std::string_view to) {
	size_t pos = 0;

	while((pos = str.find(from, pos)) != std::string::npos) {
		str.replace(pos, from.size(), to);
		pos += to.size();
	}
}

}
