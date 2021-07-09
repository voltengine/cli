#include "info_command.hpp"

#include "util/file.hpp"
#include "util/json.hpp"
#include "util/version.hpp"
#include "find_package_in_archives.hpp"

namespace fs = std::filesystem;
using namespace util;

namespace commands {

info_command::info_command() : command(
		"info",
		"{package} [{release}]",
		"Shows detailed information about package in the archives.\n"
		"Release version may be provided to list dependencies insted of releases.") {}

void info_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0)
		throw std::runtime_error("Package ID must be provided.");

	if (args.size() > 2) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n"
				  << termcolor::reset;
	}

	json manifest = find_package_in_archives(args[0]);
	std::string release = args.size() > 1 ? args[1] : "";
	if (!release.empty()) {
		util::version dummy(release);

		if (!manifest.as<json::object>().contains("releaseDependencies")   ||
				manifest["releaseDependencies"].as<json::object>().empty() ||
				!manifest["releaseDependencies"].as<json::object>().contains(release))
			throw std::runtime_error("Package was never released under that version.");
	}
	
	std::string git_url = manifest["git"];

	std::cout << '\n' << manifest["title"].as<json::string>()
			  << " (" << termcolor::bright_green
			  << manifest["id"].as<json::string>() << termcolor::reset
			  << ')' << (release.empty() ? "" : ' ' + release) << termcolor::bright_green << " @ " << termcolor::reset
			  << manifest["publisher"].as<json::string>() << '\n';

	std::cout << termcolor::bright_green << "\nGit URL:\n"
			  << termcolor::reset << git_url << '\n';

	if (manifest.as<json::object>().contains("description")) {
		std::cout << termcolor::bright_green << "\nDescription:\n"
				<< termcolor::reset << manifest["description"].as<json::string>() << '\n';
	}

	if (manifest.as<json::object>().contains("license")) {
		std::cout << termcolor::bright_green << "\nLicense:\n"
				<< termcolor::reset << manifest["license"].as<json::string>() << '\n';
	}

	if (manifest.as<json::object>().contains("releaseDependencies") &&
			!manifest["releaseDependencies"].as<json::object>().empty()) {
		if (!release.empty()) {
			json::array deps = manifest["releaseDependencies"][release];

			if (deps.empty()) {
				std::cout << termcolor::bright_green << "\nThis release has no dependencies.\n"
						  << termcolor::reset;
			} else {
				std::cout << termcolor::bright_green << "\nDependencies:\n"
						  << termcolor::reset;

				for (json &dep : deps)
					std::cout << dep.as<json::string>() << '\n';
			}
		} else {
			json::object &obj = manifest["releaseDependencies"];
			std::vector<util::version> releases;
			releases.reserve(obj.size());

			for (auto &x : obj) {
				try {
					releases.emplace_back(x.first);
				} catch (std::runtime_error &) {}
			}
			std::sort(releases.begin(), releases.end(), std::greater());

			std::cout << termcolor::bright_green << "\nReleases:\n"
					  << termcolor::reset;
			
			for (util::version &version : releases)
				std::cout << version << '\n';
		}
	}

	return;
}

}
