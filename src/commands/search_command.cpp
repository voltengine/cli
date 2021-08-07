#include "commands.hpp"

#include "util/file.hpp"
#include "util/json.hpp"
#include "util/url.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
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
	if (args.size() == 0) {
		std::cout << tc::bright_red << "Query must be provided.\n"
				  << tc::reset;
		return;
	}

	std::string url_path = "search?query=" + util::encode_url(std::accumulate(
		std::next(args.begin()), 
		args.end(), 
		args.front(), 
		[](std::string &&accumulator, std::string word) {
			return std::move(accumulator) + ' ' + word;
		}
	));

	// ID : title + description
	std::unordered_map<std::string, std::string> packages;

	json::object archives = json::parse(util::read_file(
			std::getenv("VOLT_PATH") / fs::path("config.json"))
			)["archives"];

	for (auto &archive : archives) {
		std::string url = archive.first;
		if (url.back() != '/')
			url += '/';

		std::cout << "Searching at \"" << url << "\"..." << std::endl;

		json results;
		try {
			results = json::parse(util::download(url + url_path));
		} catch (std::exception &e) {
			std::cout << tc::bright_yellow
					  << e.what() << '\n'
					  << tc::reset;
			continue;
		}

		for (json &item : results.as<json::array>()) {
			std::string &id = item;
			if (packages.contains(id))
				continue;
			
			std::string manifest_url = url + "package/" + id + '/';
			json manifest;

			try {
				manifest = json::parse(util::download(manifest_url));
			} catch (std::exception &) {
				// If manifest is unavailable, something
				// went wrong on the server.
				continue;
			}

			packages[id] = manifest["description"].as<json::string>();
		}
	}

	if (packages.size() == 0)
		throw std::runtime_error("No packages were found.");

	std::cout << "\nFound " << packages.size()
			  << (packages.size() == 1 ? " package:\n" : " packages:\n");

	for (auto &package : packages) {
		size_t i = package.first.find('/');
		std::cout << '\n'
				  << tc::bright_green << package.first.substr(0, i)
				  << tc::reset << '/'
				  << tc::bright_green << package.first.substr(i + 1)
				  << tc::reset << '\n' << package.second
				  << '\n';
	}
}

}
