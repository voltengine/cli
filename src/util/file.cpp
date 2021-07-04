#include "file.hpp"

namespace fs = std::filesystem;

namespace util {

std::string read_file(const fs::path &path) {
	std::ifstream stream(path, std::ifstream::in);
	std::string buffer;

	stream.seekg(0, std::ios::end);
	buffer.reserve(stream.tellg());
	stream.seekg(0, std::ios::beg);

	buffer.assign(
			std::istreambuf_iterator<char>(stream),
			std::istreambuf_iterator<char>());
	return buffer;
}

}
