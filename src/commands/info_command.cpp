#include "info_command.hpp"

#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;
using namespace util;

namespace commands {

info_command::info_command() : command(
		"info",
		"{package-id}",
		"Shows detailed information about a package.") {}

bool info_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0) {
		std::cout << termcolor::bright_red << "Package must be provided.\n"
				  << termcolor::reset;
		return false;
	}

	static std::regex package_name_validator("[0-9a-z-]+");

	std::string package_name = args[0];
	std::string package_url;

	if (!std::regex_match(args[0], package_name_validator)) {
		std::cout << termcolor::bright_red
					<< "Invalid package name.\n"
					<< termcolor::reset;
		return true;
	}

	fs::path volt_path(std::getenv("VOLT_PATH"));
	json archives_json = json::parse(util
			::read_file(volt_path / "archives.json"));
	auto &archives = archives_json.as<json::array>();

	for (auto &archive : archives_json.as<json::array>()) {
		std::string &url = archive.as<json::string>();
		if (url.back() != '/')
			url += '/';

		std::cout << "Checking at '" << url << "'...\n";

		url += "packages/" + args[0] + ".json";
		try {
			json package = json::parse(util::download(url, volt_path / "cacert.pem"));
			package_url = package["url"].as<json::string>();
			break;
		} catch (std::exception &e) {
			std::cout << termcolor::bright_red
						<< "Not found: " << e.what() << '\n'
						<< termcolor::reset;
		}
	}

	if (package_url.empty()) {
		std::cout << termcolor::bright_red
				  << "Package not found in archives.\n"
				  << termcolor::reset;
		return true;
	}

	json info = json::parse(util::download(package_url, volt_path / "cacert.pem"));

	std::cout << '\n' << info["title"].as<json::string>()
			  << " (" << termcolor::bright_green
			  << info["id"].as<json::string>() << termcolor::reset
			  << ") " << info["version"].as<json::string>()
			  << termcolor::bright_green << " @ " << termcolor::reset
			  << info["publisher"].as<json::string>() << '\n';

	std::cout << termcolor::bright_green << "\nGit URL:\n"
			  << termcolor::reset << info["git"].as<json::string>() << '\n';

	if (info.as<json::object>().contains("description")) {
		std::cout << termcolor::bright_green << "\nDescription:\n"
				<< termcolor::reset << info["description"].as<json::string>() << '\n';
	}

	if (info.as<json::object>().contains("dependencies") &&
			!info["dependencies"].as<json::array>().empty()) {
		std::cout << termcolor::bright_green << "\nDependencies:\n"
				<< termcolor::reset;

		for (json &dep : info["dependencies"].as<json::array>()) {
			std::cout << dep.as<json::string>() << '\n';
		}
	}

	if (info.as<json::object>().contains("license")) {
		std::cout << termcolor::bright_green << "\nLicense:\n"
				<< termcolor::reset << info["license"].as<json::string>() << '\n';
	}

	return true;
}

}
