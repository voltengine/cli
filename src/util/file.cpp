#include "file.hpp"

#include "util/http.hpp"
#include "common.hpp"

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
	fs::create_directories(path.parent_path());
	std::ofstream stream(path, std::ofstream::out);
	stream << str;
}

std::string download(std::string_view url) {
	std::string buffer;

	http request;
	request.set_certificate(common::getenv("VOLT_PATH") / fs::path("cacert.pem"));
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

int32_t shell(std::string cmd, const std::function<void(std::string_view)>
		&stdout_cb, bool redirect_stderr, size_t buffer_capacity) {
	std::vector<char> buffer;
	buffer.resize(buffer_capacity);

	if (redirect_stderr)
		cmd += " 2>&1";

#ifdef _WIN32
	cmd = '"' + cmd + '"'; // _popen strips quotes
	FILE *pipe = _popen(cmd.c_str(), "r");
#else
	FILE *pipe = popen(cmd.c_str(), "r");
#endif

	if (!pipe)
		throw std::runtime_error("Failed to execute command.");
	
	while (std::fgets(buffer.data(), buffer.size(), pipe) != nullptr)
		stdout_cb(buffer.data());

#ifdef _WIN32
	return _pclose(pipe);
#else
	return WEXITSTATUS(pclose(pipe));
#endif
}

}
