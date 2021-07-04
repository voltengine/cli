#include "commands/info_command.hpp"

#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;

namespace commands {

info_command::info_command() : command(
		"info",
		"({git-url} | {package-id}) [{version}]",
		"Shows details about the latest release of a package.\n"
		"Can optionally inspect specified version.") {}

bool info_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0) {
		std::cout << termcolor::bright_red << "Package must be provided.\n"
				  << termcolor::reset;
		return false;
	}

	// URI Validator by @stephenhay
	std::regex uri_validator("^(https?|ftp)://[^\\s/$.?#].[^\\s]*$");

	std::string url = args[0];

	if (!std::regex_match(url, uri_validator)) {
		fs::path volt_path(std::getenv("VOLT_PATH"));
		fs::path archives_path = volt_path / "archives/";

		util::json archives_json = util::json::parse(util
				::read_file(volt_path / "archives.json"));
		auto &archives = archives_json.as<util::json::array>();

		for (size_t i = 0; i < archives.size(); i++) {
			std::string &url = archives[i].as<util::json::string>();
			if (url.back() != '/')
				url += '/';
			
			std::string packages_url = url + "packages.json";
			std::string hash_url = packages_url + ".sha256";
		}
	}

	return true;
}

}
