#include "install_command.hpp"

#include "util/file.hpp"
#include "util/http.hpp"
#include "util/json.hpp"
#include "util/version.hpp"
#include "find_package_in_archives.hpp"

namespace fs = std::filesystem;
using namespace util;

namespace commands {

install_command::install_command() : command(
		"install",
		"",
		"Checks for dependency conflicts and downloads missing components.") {}

void install_command::run(const std::vector<std::string> &args) const {
	// Validation

	if (args.size() > 0) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n"
				  << termcolor::reset;
	}

	fs::path package_path = fs::current_path() / "package.json";

	if (!fs::exists(package_path)) {
		std::cout << termcolor::bright_red
				  << "No 'package.json' in this directory.\n"
				  << termcolor::reset;
		return;
	}

	// // To detect circular references
	// std::vector<std::string> visited_packages;
	std::stack<json> packages_to_check;
	// Dependency ID + (Dependent's ID + Dependency version)
	std::unordered_map<std::string, std::tuple<std::string, util::version>> dependencies;

	packages_to_check.push(json::parse(read_file(package_path)));

	while (!packages_to_check.empty()) {
		json package = packages_to_check.top();
		packages_to_check.pop();

		std::cout << package["id"] << std::endl;

		std::string &package_id = package["id"];
		// visited_packages.push_back(package_id);

		if (!package.as<json::object>().contains("dependencies"))
			continue;

		for (json &dep : package["dependencies"].as<json::array>()) {
			auto tokens = util::split(dep.as<json::string>(), ":", true);
			std::string &id = tokens[0];

			if (tokens.size() != 2) {
				std::cout << termcolor::bright_red
						  << "Dependency has no version: " << id << "\n"
						  << termcolor::reset;
				return;
			}

			util::version new_version = tokens[1];

			// Update version
			if (dependencies.contains(id)) {
				auto &dependency = dependencies.at(id);
				std::string current_dependent = std::get<0>(dependency);
				util::version current_version = std::get<1>(dependency);

				if (new_version.major != current_version.major) {
					std::cout << termcolor::bright_red
							  << "\nDependency conflict detected: " << id << "\n'"
							  << package_id << "' requires " << new_version << ", but '"
							  << current_dependent << "' already depends on " << current_version
							  << '.' << termcolor::reset;
					return;
				}

				if (new_version > current_version)
					dependencies.emplace(id, std::make_tuple(package_id, new_version));
			} else
				dependencies.emplace(id, std::make_tuple(package_id, new_version));

			// if (std::find(visited_packages.begin(), visited_packages.end(),
			// 	id) != visited_packages.end()) {
			// 	std::cout << termcolor::bright_red
			// 			  << "\nFound circular dependency: " << id << '\n'
			// 			  << termcolor::reset;
			// 	return;
			// }

			std::cout << termcolor::bright_green
					  << "Fetching dependency: " << id << '\n'
					  << termcolor::reset;
			packages_to_check.push(find_package_in_archives(id));
		}
	}
}

}
