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

	std::string manifest_url;

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
			json package = json::parse(util::download(url, cert_path));
			manifest_url = package["manifest"].as<json::string>();
			break;
		} catch (std::exception &e) {
			std::cout << termcolor::bright_yellow
					  << "Not found: " << e.what() << '\n'
					  << termcolor::reset;
		}
	}

	if (manifest_url.empty())
		throw std::runtime_error("Package not found in archives.");

	return json::parse(util::download(manifest_url, cert_path));
}
