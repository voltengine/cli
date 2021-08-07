#include "commands.hpp"

#include "util/file.hpp"
#include "util/http.hpp"
#include "util/json.hpp"
#include "util/version.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
using namespace util;

namespace commands {

install_command::install_command() : command(
		"install",
		"[[{scope}/]{name}[ {major}.{minor}]]",
		"Checks for dependency conflicts and downloads missing components.\n"
		"Optionally adds a dependency to \"package.json\".") {}

void install_command::run(const std::vector<std::string> &args) const {
	if (args.size() > 2) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n"
				  << termcolor::reset;
	}

	if (args.size() > 1) {
		static const std::regex validator("^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)$");
		if (!std::regex_match(args[1], validator))
			throw std::runtime_error("Invalid version. Please, specify only major and minor components.");
	}

	fs::path package_path = fs::current_path() / "package.json";

	if (!fs::exists(package_path))
		throw std::runtime_error("No \"package.json\" in this directory.");

	json package = json::parse(read_file(package_path));

	// If package ID was specified
	if (args.size() > 0) {
		std::string id = common::prepend_default_scope(args[0]);
		json::object releases = common::find_manifest_in_archives(id)["releases"];
		if (releases.empty())
			throw std::runtime_error("Dependency has no releases.");

		static const std::regex parser("^\\d+\\.\\d+");

		std::string release;
		if (args.size() > 1) {
			release = args[1];

			if (std::find_if(releases.begin(), releases.end(), [&release](auto &item) {
				std::smatch match;
				std::regex_search(item.first, match, parser);
				return match.str() == release;
			}) == releases.end())
				throw std::runtime_error("No matching releases found.");
		} else {
			std::string latest = std::max_element(releases.begin(),
					releases.end(), [](auto &a, auto &b) {
				return util::version(a.first) < util::version(b.first);
			})->first;
			std::smatch match;
			std::regex_search(latest, match, parser);
			release = match.str();
		}

		package["dependencies"][id] = release;
		util::write_file(package_path, util::to_string(package));
		std::cout << tc::bright_green << "\nFile has been written:\n"
				  << tc::reset << package_path.string() << '\n';
	}

	std::cout << "Dependency validation and download are not yet implemented.";

	// // // To detect circular references
	// // std::vector<std::string> visited_packages;
	// std::stack<json> packages_to_check;
	// // Dependency ID + (Dependent's ID + Dependency version)
	// std::unordered_map<std::string, std::tuple<std::string, util::version>> dependencies;

	// packages_to_check.push(json::parse(read_file(package_path)));

	// while (!packages_to_check.empty()) {
	// 	json package = packages_to_check.top();
	// 	packages_to_check.pop();

	// 	std::cout << package["id"] << std::endl;

	// 	std::string &package_id = package["id"];
	// 	// visited_packages.push_back(package_id);

	// 	if (!package.contains("dependencies"))
	// 		continue;

	// 	for (json &dep : package["dependencies"].as<json::array>()) {
	// 		auto tokens = util::split(dep.as<json::string>(), ":", true);
	// 		std::string &id = tokens[0];

	// 		if (tokens.size() != 2) {
	// 			std::cout << termcolor::bright_red
	// 					  << "Dependency has no version: " << id << "\n"
	// 					  << termcolor::reset;
	// 			return;
	// 		}

	// 		util::version new_version = tokens[1];

	// 		// Update version
	// 		if (dependencies.contains(id)) {
	// 			auto &dependency = dependencies.at(id);
	// 			std::string current_dependent = std::get<0>(dependency);
	// 			util::version current_version = std::get<1>(dependency);

	// 			if (new_version.major != current_version.major) {
	// 				std::cout << termcolor::bright_red
	// 						  << "\nDependency conflict detected: " << id << '\n'
	// 						  << package_id << " requires " << new_version << ", but "
	// 						  << current_dependent << " already depends on " << current_version
	// 						  << '.' << termcolor::reset;
	// 				return;
	// 			}

	// 			if (new_version > current_version)
	// 				dependencies.emplace(id, std::make_tuple(package_id, new_version));
	// 		} else
	// 			dependencies.emplace(id, std::make_tuple(package_id, new_version));

	// 		// if (std::find(visited_packages.begin(), visited_packages.end(),
	// 		// 	id) != visited_packages.end()) {
	// 		// 	std::cout << termcolor::bright_red
	// 		// 			  << "\nFound circular dependency: " << id << '\n'
	// 		// 			  << termcolor::reset;
	// 		// 	return;
	// 		// }

	// 		std::cout << termcolor::bright_green
	// 				  << "Fetching dependency: " << id << '\n'
	// 				  << termcolor::reset;
	// 		packages_to_check.push(common::find_manifest_in_archives(id));
	// 	}
	// }
}

}
