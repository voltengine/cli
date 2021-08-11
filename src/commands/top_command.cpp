#include "commands.hpp"

#include "util/file.hpp"
#include "colors.hpp"
#include "common.hpp"

namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

namespace commands {

top_command::top_command() : command(
		"top",
		"",
		"Displays top 10 packages of the week in an archive.") {}

void top_command::run(const std::vector<std::string> &args) const {
	std::string url = common::select_archive();

	std::cout << "Fetching top packages...\n";

	nl::json packages = nl::json::parse(util::download(url + "top/"));

	if (packages.empty())
		throw std::runtime_error("No packages were found.");

	for (nl::json &package : packages) {
		auto &id = package.get_ref<nl::json::string_t &>();
		std::string manifest_url = url + "package/" + id + '/';

		nl::json manifest;
		try {
			manifest = nl::json::parse(util::download(manifest_url));
		} catch (...) {
			continue;
		}

		auto &views = manifest["views"].get_ref<nl::json::object_t &>();
		uint32_t weekly_views = 0;
		date::sys_days today = date::floor<date::days>(std::chrono::system_clock::now()); 
		
		for (auto &view : views) {
			date::sys_days record_day;
			std::istringstream(view.first) >> date::parse("%F", record_day);
			if ((today - record_day).count() < 7)
				weekly_views += view.second.get<uint32_t>();
		}

		size_t i = id.find('/');
		std::cout << '\n'
				  << colors::main << id.substr(0, i)
				  << tc::reset << '/'
				  << colors::main << id.substr(i + 1)
				  << tc::reset << " (" << colors::main
				  << weekly_views << tc::reset << " View"
				  << (weekly_views == 1 ? ")\n" : "s)\n")
				  << manifest["description"].get_ref<nl::json::string_t &>()
				  << '\n';
	}
}

}
