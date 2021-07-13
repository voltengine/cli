#include "find_package_in_archives.hpp"

#include "util/file.hpp"
#include "util/http.hpp"

namespace fs = std::filesystem;
using namespace util;

json find_package_in_archives(std::string_view id) {
	static std::regex package_name_validator("[0-9a-z-]+");

	if (!std::regex_match(std::string(id), package_name_validator)) {
		throw std::runtime_error("Invalid package name.");
	}

	json package;

	fs::path volt_path = std::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";

	json archives_json = json::parse(util
			::read_file(volt_path / "archives.json"));
	json::array &archives = archives_json;

	for (auto &archive : archives_json.as<json::array>()) {
		std::string &url = archive;
		if (url.back() != '/')
			url += '/';

		std::cout << "Checking at '" << url << "'...\n";

		url += "packages/" + std::string(id) + ".json";
		try {
			package = json::parse(util::download(url, cert_path));
			break;
		} catch (std::exception &e) {
			std::cout << termcolor::bright_yellow
					  << "Not found: " << e.what() << '\n'
					  << termcolor::reset;
		}
	}

	if (package.is<json::null>())
		throw std::runtime_error("Package not found in archives.");

	return package;
}
