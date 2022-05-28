#include "commands.hpp"

#include "util/file.hpp"
#include "util/url.hpp"
#include "colors.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

// [{keyword, [package]}] 
using search_index = std::vector<std::tuple<std::string, std::vector<std::string>>>;

static search_index get_search_index(std::string_view url);

namespace commands {

search_command::search_command() : command(
		"search",
		"{query...}",
		"Searches remote archives for packages.") {}

void search_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0)
		throw std::runtime_error("Query must be provided.");

	std::string url_path = "search?query=" + util::encode_url(std::accumulate(
		std::next(args.begin()), 
		args.end(), 
		args.front(), 
		[](std::string &&accumulator, auto &word) {
			return std::move(accumulator) + ' ' + word;
		}
	));

	// ID : title + description
	std::unordered_map<std::string, std::string> packages;

	nl::json::object_t archives = nl::json::parse(util::read_file(
			common::getenv("VOLT_PATH") / fs::path("config.json"))
			)["archives"];

	for (auto &archive : archives) {
		std::string url = archive.first;
		if (url.back() != '/')
			url += '/';

		std::cout << "Searching at \"" << url << "\"..." << std::endl;

		nl::json results;
		try {
			results = nl::json::parse(util::download(url + url_path));
		} catch (std::exception &e) {
			std::cout << colors::warning
			          << e.what() << '\n'
			          << tc::reset;
			continue;
		}

		for (nl::json &item : results) {
			std::string id = item;
			if (packages.contains(id))
				continue;
			
			std::string manifest_url = url + "package/" + id + '/';
			nl::json manifest;

			try {
				manifest = nl::json::parse(util::download(manifest_url));
			} catch (...) {
				// If manifest is unavailable, something
				// went wrong on the server.
				continue;
			}

			packages[id] = manifest["description"];
		}
	}

	if (packages.empty())
		throw std::runtime_error("No packages were found.");

	std::cout << colors::success << "\nFound " << packages.size()
	          << (packages.size() == 1 ? " package:\n" : " packages:\n")
	          << tc::reset;

	for (auto &package : packages) {
		size_t i = package.first.find('/');
		std::cout << '\n'
		          << colors::main << package.first.substr(0, i)
		          << tc::reset << '/'
		          << colors::main << package.first.substr(i + 1)
		          << tc::reset << '\n' << package.second
		          << '\n';
	}
}

}
