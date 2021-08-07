#include "commands.hpp"

#include "util/file.hpp"
#include "util/json.hpp"
#include "util/version.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
using namespace util;

namespace commands {

info_command::info_command() : command(
		"info",
		"[{scope}/]{name}[ {release}]",
		"Shows detailed information about package in the archives.\n"
		"Release version may be provided to list dependencies insted of releases.") {}

void info_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0)
		throw std::runtime_error("Package ID must be provided.");

	if (args.size() > 2) {
		std::cout << tc::bright_yellow << "Ignoring extra arguments.\n"
				  << tc::reset;
	}

	std::string id = common::prepend_default_scope(args[0]);
	json manifest = common::find_manifest_in_archives(id);

	std::string release = args.size() > 1 ? args[1] : "";
	if (!release.empty()) {
		util::version dummy(release);

		if (!manifest["releases"].contains(release))
			throw std::runtime_error("No such release.");
	}
	
	std::string git_url = manifest["git"];

	size_t slash_index = id.find('/');
	std::cout << '\n'
			  << tc::bright_green << id.substr(0, slash_index)
			  << tc::reset << '/'
			  << tc::bright_green << id.substr(slash_index + 1)
			  << tc::reset
			  << (release.empty() ? "" : ' ' + release) << '\n'
			  << manifest["description"].as<json::string>() << '\n';

	std::cout << tc::bright_green << "\nGit URL: "
			  << tc::reset << git_url << '\n';

	std::cout << tc::bright_green << "License: "
			  << tc::reset << manifest["license"].as<json::string>() << '\n';
	
	json::array &keywords = manifest["keywords"];
	if (!keywords.empty()) {
		std::cout << tc::bright_green << "Keywords: "
				  << tc::reset;

		std::cout << keywords.front().as<json::string>();

		auto it = keywords.begin();
		while (++it != keywords.end())
			std::cout << ", " << it->as<json::string>();

		std::cout << '\n';
	}

	json::object &views = manifest["views"];
	uint32_t weekly_views = 0;
	date::sys_days today = date::floor<date::days>(std::chrono::system_clock::now()); 
	
	for (auto &view : views) {
		date::sys_days record_day;
    	std::istringstream(view.first) >> date::parse("%F", record_day);
		if ((today - record_day).count() < 7)
			weekly_views += std::round(view.second.as<json::number>());
	}

	std::cout << tc::bright_green << "Weekly Views: "
			  << tc::reset << weekly_views << '\n';

	if (!release.empty()) {
		std::string &released = manifest["releases"][release]["created"];
		std::cout << tc::bright_green << "\nReleased: "
				  << tc::reset << released << '\n';

		json::object &deps = manifest["releases"][release]["dependencies"];

		if (deps.empty()) {
			std::cout << tc::bright_green << "\nThis release has no dependencies.\n"
					  << tc::reset;
		} else {
			std::cout << tc::bright_green << "\nDependencies:\n"
					  << tc::reset;

			for (auto &dep : deps) {
				slash_index = dep.first.find('/');
				std::cout << '\n'
						  << tc::bright_green << dep.first.substr(0, slash_index)
						  << tc::reset << '/'
						  << tc::bright_green << dep.first.substr(slash_index + 1)
						  << tc::reset << ' '
						  << dep.second.as<json::string>() << '\n';
			}
		}
	} else {
		std::cout << tc::bright_green << "\nCreated: "
				  << tc::reset << manifest["created"].as<json::string>() << '\n';
		std::cout << tc::bright_green << "Modified: "
				  << tc::reset << manifest["modified"].as<json::string>() << '\n';

		json::object &obj = manifest["releases"];

		if (obj.empty()) {
			std::cout << tc::bright_green << "\nThis package has no releases.\n"
					  << tc::reset;
		} else {
			std::vector<util::version> releases;
			releases.reserve(obj.size());

			for (auto &x : obj)
				releases.emplace_back(x.first);
			std::sort(releases.begin(), releases.end(), std::greater());

			std::cout << tc::bright_green << "\nReleases:\n"
					<< tc::reset;
			
			for (util::version &version : releases)
				std::cout << version << '\n';
		}
	}
}

}
