#include "commands.hpp"

#include "util/file.hpp"
#include "util/http.hpp"
#include "util/version.hpp"
#include "util/url.hpp"
#include "colors.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

namespace commands {

unpublish_command::unpublish_command() : command(
		"unpublish",
		"{id} [{version}]",
		"Removes package or its single version from selected archive.") {}

void unpublish_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0)
		throw std::runtime_error("Package ID must be provided.");

	if (args.size() > 2) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n\n"
				  << termcolor::reset;
	}

	// Throws if argument is invalid
	if (args.size() > 1)
		util::version(args[1]);

	std::string id = common::get_valid_id(args[0]);

	static const std::regex id_validator(
			"(?=^.{1,39}\\/.{1,64}$)^([a-z\\d]+(-[a-z\\d]+)*)\\/"
			"([a-z][a-z\\d]*(-[a-z\\d]+)*)$");
	if (!std::regex_match(id, id_validator))
		throw std::runtime_error("Invalid package ID.");

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

	fs::path volt_path = std::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";

	std::string buffer;
	http request;
	bool succeed;

	request.set_certificate(cert_path);
	request.set_method(http::method::del);
	request.set_url(url + "package/" + id + '/' + (args.size() > 1
			? "?release=" + util::encode_url(args[1]) : ""));
	request.set_header("Authorization", "Bearer " + token);
	request.on_response([&succeed](const auto &response) {
		if (response.status == 200) {
			succeed = true;
		} else if (response.status == 400 || response.status == 401
				|| response.status == 403) {
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
