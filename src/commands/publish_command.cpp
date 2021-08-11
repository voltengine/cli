#include "commands.hpp"

#include "util/file.hpp"
#include "util/http.hpp"
#include "colors.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

namespace commands {

publish_command::publish_command() : command(
		"publish",
		"",
		"Submits \"package.json\" to archive."
		"Might request authentication if credentials are outdated.") {}

void publish_command::run(const std::vector<std::string> &args) const {
	fs::path volt_path = std::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";
	fs::path package_path = fs::current_path() / "package.json";

	if (!fs::exists(package_path))
		throw std::runtime_error("No \"package.json\" in this directory.");

	std::string url = common::select_archive();
	std::string token = common::get_cached_token(url);

	std::cout << "Validating token...";
	nl::json user;
	try {
		user = common::get_user_info(token);
	} catch (std::exception &e) {
		std::cout << colors::warning << " Failed.\n" << tc::reset;
		throw e;
	}

	if (user.is_null()) {
		std::cout << colors::warning << " Failed.\n\n" << tc::reset;
		token = common::authorize(url).token;
	} else
		std::cout << colors::success << " Success.\n" << tc::reset;
	
	std::string buffer;
	http request;
	bool succeed;

	request.set_certificate(cert_path);
	request.set_method(http::method::post);
	request.set_url(url + "package/");
	request.set_header("Content-Type", "application/json");
	request.set_header("Authorization", "Bearer " + token);
	request.set_body(util::read_file(package_path));
	request.on_response([&succeed](const auto &response) {
		if (response.status == 200 || response.status == 201) {
			succeed = true;
		} else if (response.status == 400 || response.status == 401
				|| response.status == 403 || response.status == 503) {
			succeed = false;
		} else {
			throw http::error("Remote returned " +
					std::to_string(response.status) + ".");
		}
	});
	request.on_data([&buffer](const auto &data) {
		buffer.insert(buffer.end(), data.begin(), data.end());
	});

	request.send();

	std::cout << (succeed ? colors::success : colors::error);
	std::cout << '\n' << buffer << '\n' << tc::reset;
}

}
