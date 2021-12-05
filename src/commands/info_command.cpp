#include "commands.hpp"

#include "util/date.hpp"
#include "util/file.hpp"
#include "util/version.hpp"
#include "colors.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;
using date::operator<<;

namespace commands {

info_command::info_command() : command(
		"info",
		"[{id} [{version}]]",
		"Shows detailed information about package in the archives.\n"
		"Release version may be provided to list dependencies insted of releases.") {}

void info_command::run(const std::vector<std::string> &args) const {
	if (args.size() > 2) {
		std::cout << colors::warning << "Ignoring extra arguments.\n\n"
		          << tc::reset;
	}

	std::string id, release = args.size() > 1 ? args[1] : "";;
	nl::json manifest;
	bool fake_manifest;
	if (fake_manifest = args.size() == 0) {
		fs::path package_path = fs::current_path() / "package.json";
		if (!fs::exists(package_path))
			throw std::runtime_error("No \"package.json\" in current directory.");

		manifest = nl::json::parse(util::read_file(package_path));
		id = manifest["id"];
		release = manifest["version"];
		manifest["releases"][release]["dependencies"] = manifest["dependencies"];
	} else {
		id = common::get_valid_id(args[0]);
		static const std::regex id_validator(
				"(?=^.{1,39}\\/.{1,64}$)^([a-z\\d]+(-[a-z\\d]+)*)\\/"
				"([a-z][a-z\\d]*(-[a-z\\d]+)*)$");

		if (!std::regex_match(id, id_validator))
			throw std::runtime_error("Invalid package ID.");

		manifest = common::find_manifest_in_archives(id);
		std::cout << '\n';
	}

	if (!release.empty()) {
		util::version dummy(release);

		if (!manifest["releases"].contains(release))
			throw std::runtime_error("No such release.");
	}
	
	std::string git_url = manifest["git"];

	size_t slash_index = id.find('/');
	std::cout << colors::main << id.substr(0, slash_index)
	          << tc::reset << '/'
	          << colors::main << id.substr(slash_index + 1)
	          << tc::reset
	          << (release.empty() ? "" : ' ' + release) << '\n'
	          << manifest["description"].get_ref<nl::json::string_t &>() << '\n';

	std::cout << colors::main << "\nGit URL: "
	          << tc::reset << git_url << '\n';

	std::cout << colors::main << "License: "
	          << tc::reset << manifest["license"].get_ref<nl::json::string_t &>() << '\n';
	
	auto &keywords = manifest["keywords"].get_ref<nl::json::array_t &>();
	if (!keywords.empty()) {
		std::cout << colors::main << "Keywords: "
		          << tc::reset;

		std::cout << keywords.front().get_ref<nl::json::string_t &>();

		auto it = keywords.begin();
		while (++it != keywords.end())
			std::cout << ", " << it->get<std::string>();

		std::cout << '\n';
	}

	if (!fake_manifest) {
		auto &views = manifest["views"].get_ref<nl::json::object_t &>();
		uint32_t weekly_views = 0;
		date::sys_days today = date::floor<date::days>(std::chrono::system_clock::now()); 
		
		for (auto &view : views) {
			date::sys_days record_day;
			std::istringstream(view.first) >> date::parse("%F", record_day);
			if ((today - record_day).count() < 7)
				weekly_views += view.second.get<uint32_t>();
		}

		std::cout << colors::main << "Weekly Views: "
				<< tc::reset << weekly_views << '\n';
	}

	if (!release.empty()) {
		if (!fake_manifest) {
			date::sys_time<std::chrono::milliseconds> released
					= util::parse_iso_date(manifest
					["releases"][release]["created"]);

			std::cout << colors::main << "\nReleased: "
					<< tc::reset << released << '\n';
		}

		auto &deps = manifest["releases"][release]
				["dependencies"].get_ref<nl::json::object_t &>();

		if (deps.empty()) {
			std::cout << colors::main << "\nThis release has no dependencies.\n"
			          << tc::reset;
		} else {
			std::cout << colors::main << "\nDependencies:\n"
			          << tc::reset;

			for (auto &dep : deps) {
				slash_index = dep.first.find('/');
				std::cout << "- "
				          << colors::main << dep.first.substr(0, slash_index)
				          << tc::reset << '/'
				          << colors::main << dep.first.substr(slash_index + 1)
				          << tc::reset << ' '
				          << dep.second.get_ref<nl::json::string_t &>() << '\n';
			}
		}
	} else {
		date::sys_time<std::chrono::milliseconds> created
				= util::parse_iso_date( manifest["created"]);
		date::sys_time<std::chrono::milliseconds> modified
				= util::parse_iso_date( manifest["modified"]);

		std::cout << colors::main << "\nCreated: "
		          << tc::reset << created << '\n';
		std::cout << colors::main << "Modified: "
		          << tc::reset << modified << '\n';

		auto &obj = manifest["releases"].get_ref<nl::json::object_t &>();

		if (obj.empty()) {
			std::cout << colors::main << "\nThis package has no releases.\n"
			          << tc::reset;
		} else {
			std::vector<util::version> releases;
			releases.reserve(obj.size());

			for (auto &x : obj)
				releases.emplace_back(x.first);
			std::sort(releases.begin(), releases.end(), std::greater());

			std::cout << colors::main << "\nReleases:\n"
					<< tc::reset;
			
			for (util::version &version : releases)
				std::cout << "- " << version << '\n';
		}
	}
}

}
