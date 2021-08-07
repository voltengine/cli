#include "url.hpp"

namespace util {

std::string encode_url(std::string_view content) {
	std::stringstream ss;
	ss << std::hex;

	auto it = content.begin();
	while (it != content.end()) {
		const char &c = *it;
		//A-Z a-z 0-9 - _ . ! ~ * ' ( )
		if (std::isalpha(c) || std::isdigit(c) || c == '-'
				|| c == '_' || c == '.' || c == '!' || c == '~'
				|| c == '*' || c == '\'' || c == '(' || c == ')') {
			ss << c;
		} else if (c == ' ')
			ss << '+';
		else {
			ss << '%' << std::uppercase
			   << static_cast<uint32_t>(c) << std::nouppercase;
		}
		it++;
	}

	return ss.str();
}

}
