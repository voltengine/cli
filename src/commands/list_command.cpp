#include "commands.hpp"

#include "util/file.hpp"
#include "colors.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;

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
		std::cout << colors::warning << "Ignoring extra arguments.\n\n"
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

					nl::json json = nl::json::parse(util::read_file(json_path));

					if (json["id"] != pkg.get_id())
						continue;

					if (json["version"] != pkg.version)
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
		std::cout << colors::main << pkg.scope
		          << tc::reset << '/'
		          << colors::main << pkg.name
		          << ' ' << pkg.version
		          << tc::reset << '\n';
	}
}

}
