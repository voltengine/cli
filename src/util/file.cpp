#include "file.hpp"

#include "http.hpp"

namespace fs = std::filesystem;

namespace util {

std::string read_file(const fs::path &path) {
	if (!fs::exists(path))
		throw std::runtime_error("File not found:\n" + path.string());

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

std::string download(std::string_view url) {
	std::string buffer;

	http request;
	request.set_certificate(std::getenv("VOLT_PATH") / fs::path("cacert.pem"));
	request.set_url(url);
	request.on_response([](const http::response &response) {
		if (response.status != 200) {
			throw http::error("Remote returned " +
					std::to_string(response.status) + ".");
		}
	});
	request.on_data([&buffer](const http::buffer &data) {
		buffer.insert(buffer.end(), data.begin(), data.end());
	});
	request.send();

	return buffer;
}

void shell(std::string cmd, const std::function<void(
		std::string_view)> &stdout_cb, size_t buffer_capacity) {
    std::vector<char> buffer;
	buffer.resize(buffer_capacity);

	cmd += " 2>&1";
#if _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
#endif

    if (!pipe)
        throw std::runtime_error("Failed to execute command.");
	
    while (std::fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        stdout_cb(buffer.data());
}

}
