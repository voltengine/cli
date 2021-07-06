#include "file.hpp"

#include "http.hpp"

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

void write_file(const fs::path &path, std::string_view str) {
	std::ofstream stream(path, std::ofstream::out);
    stream << str;
}

std::string download(std::string_view url,
		const fs::path &https_certificate) {
	std::string_view protocol = url.substr(0, 4);

	if (protocol == "http") {
		http::buffer buffer;
		http request;

		if (!https_certificate.empty())
			request.set_certificate(https_certificate);

		request.set_url(url);
		request.on_response([](const http::response &response) {
			if (response.code != 200) {
				throw http::error("Remote returned " +
						std::to_string(response.code) + ".");
			}
		});
		request.on_data([&buffer](const http::buffer &data) {
			buffer.insert(buffer.end(), data.begin(), data.end());
		});
		request.send();

		return std::string(buffer.begin(), buffer.end());
	} else if (protocol == "file")
		return read_file(url.substr(8));
	else
		throw std::runtime_error("Unknown protocol.");
}

}
