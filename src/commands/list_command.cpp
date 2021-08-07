#include "commands.hpp"

#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;

class package {
public:
	std::string scope, name, version;

	std::string get_id();
};

std::string package::get_id() {
	return scope + '/' + name;
}

namespace commands {

list_command::list_command() : command(
		"list",
		"",
		"Displays installed packages.") {}

void list_command::run(const std::vector<std::string> &args) const {
	if (args.size() > 0) {
		std::cout << tc::bright_yellow << "Ignoring extra arguments.\n"
				  << tc::reset;
	}

	fs::path packages_path = std::getenv("VOLT_PATH") / fs::path("packages/");
	std::vector<package> packages;

	if (fs::exists(packages_path) && !fs::is_empty(packages_path)) {
		for (auto &item : fs::directory_iterator(packages_path)) {
			if (!item.is_directory())
				continue;

			package pkg;

			auto &scope_path = item.path();
			pkg.scope = scope_path.filename().string();

			for (auto &item : fs::directory_iterator(scope_path)) {
				if (!item.is_directory())
					continue;

				auto &package_path = item.path();
				pkg.name = package_path.filename().string();

				for (auto &item : fs::directory_iterator(package_path)) {
					if (!item.is_directory())
						continue;

					const fs::path &version_path = item.path();
					pkg.version = version_path.filename().string();
					fs::path json_path = version_path / "package.json";

					if (!fs::exists(json_path))
						continue;

					util::json json = util::json::parse(util::read_file(json_path));

					if (json["id"].as<util::json::string>() != pkg.get_id())
						continue;

					if (json["version"].as<util::json::string>() != pkg.version)
						continue;

					packages.push_back(pkg);
				}
			}
		}
	}

	if (packages.size() == 0)
		throw std::runtime_error("No packages are installed.");

	std::cout << "Found " << packages.size() <<
			(packages.size() == 1 ? " installed package" : " installed packages") << ":\n\n";

	for (package &pkg : packages) {
		std::cout << pkg.scope << '/'
				  << tc::bright_green
				  << pkg.name << ' ' << pkg.version
				  << tc::reset << '\n';
	}
}

}
