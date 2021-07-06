#include "list_command.hpp"

#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;

struct package {
	std::string id;
	std::vector<std::string> versions;
};

namespace commands {

list_command::list_command() : command(
		"list",
		"",
		"Displays installed packages.") {}

bool list_command::run(const std::vector<std::string> &args) const {
	fs::path packages_path = std::getenv("VOLT_PATH") / fs::path("packages/");
	std::vector<package> packages;

	if (fs::exists(packages_path) && !fs::is_empty(packages_path)) {
		for (auto &item : fs::directory_iterator(packages_path)) {
			if (!item.is_directory())
				continue;
			
			package pkg;

			auto &package_path = item.path();
			pkg.id = package_path.filename().string();

			for (auto &item : fs::directory_iterator(package_path)) {
				if (!item.is_directory())
					continue;

				const fs::path &version_path = item.path();
				std::string version_str = version_path.filename().string();
				fs::path json_path = version_path / "package.json";

				if (!fs::exists(json_path))
					continue;

				util::json json = util::json::parse(util::read_file(json_path));

				if (json["id"].as<util::json::string>() != pkg.id)
					continue;

				if (json["version"].as<util::json::string>() != version_str)
					continue;

				pkg.versions.push_back(version_str);
			}

			if (pkg.versions.size() > 0)
				packages.push_back(pkg);
		}
	}

	if (packages.size() == 0) {
		std::cout << termcolor::bright_red << "No packages are installed.\n"
				  << termcolor::reset;
		return true;
	}

	std::cout << "Found " << packages.size() <<
			(packages.size() == 1 ? " package" : " packages") << ":\n\n";

	for (package &pkg : packages) {
		std::cout << termcolor::bright_green << pkg.id << '\n'
				  << termcolor::reset;
		
		for (std::string &version : pkg.versions)
			std::cout << '\t' << version << '\n';
	}

	return true;
}

}
