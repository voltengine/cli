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
	if (args.size() != 0) {
		std::cout << colors::warning << "Ignoring extra arguments.\n\n"
		          << tc::reset;
	}

	fs::path volt_path = std::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";
	fs::path package_path = fs::current_path() / "package.json";

	if (!fs::exists(package_path))
		throw std::runtime_error("No \"package.json\" in current directory.");

	std::string data = util::read_file(package_path);
	auto package = nl::json::parse(data);

	std::cout << "Checking Git tag...";

	try {
		std::string line;
		bool tag_exists = false;
		util::shell("git ls-remote --tags " + package["git"].get_ref<nl::json::string_t &>(),
				[&line, &package, &tag_exists](std::string_view out) {
			size_t i = out.find('\n');
			if (i == std::string::npos)
				line += out;
			else {
				line.insert(line.end(), out.begin(), out.begin() + i);

				if (line.find("fatal: ") != std::string::npos
						|| line.size() <= 51 || line.substr(40, 11) != "\trefs/tags/") {
					throw std::runtime_error("Could not list tags in repository:\n"
							+ package["git"].get_ref<nl::json::string_t &>());
				}

				if (line.substr(51) == package["version"])
					tag_exists = true;

				line = out.substr(i + 1);
			}
		});

		if (!tag_exists) {
			throw std::runtime_error("No " + package["version"].get_ref<nl::json::string_t &>()
					+ " tag in repository:\n" + package["git"].get_ref<nl::json::string_t &>());
		}
	} catch (std::exception &e) {
		std::cout << colors::error << " Failed.\n" << tc::reset;
		throw e;
	}

	std::cout << colors::success << " Success.\n" << tc::reset;

	std::string url = common::select_archive();
	std::string token = common::get_cached_token(url);

	std::cout << "Validating token...";
	nl::json user;
	try {
		user = common::get_user_info(token);
	} catch (std::exception &e) {
		std::cout << colors::error << " Failed.\n" << tc::reset;
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
	request.set_body(data);
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
