#include "search_command.hpp"

#include "util/crypto.hpp"
#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;
using namespace util;

static json get_search_index(std::string_view url);

namespace commands {

search_command::search_command() : command(
		"search",
		"{keywords}",
		"Searches remote archives for packages.") {}

bool search_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0) {
		std::cout << termcolor::bright_red << "Keywords must be provided.\n"
				  << termcolor::reset;
		return false;
	}

	// ID : title + version + description
	std::unordered_map<std::string, std::tuple<
			std::string, std::string, std::string>> packages;

	fs::path volt_path(std::getenv("VOLT_PATH"));
	json archives_json = json::parse(util
			::read_file(volt_path / "archives.json"));
	auto &archives = archives_json.as<json::array>();

	for (auto &archive : archives_json.as<json::array>()) {
		std::string &url = archive.as<json::string>();
		if (url.back() != '/')
			url += '/';

		std::cout << "Searching at '" << url << "'...\n";

		json index = get_search_index(url);
		json::array &index_array = index.as<json::array>();

		for (const std::string &keyword : args) {
			int32_t step = index_array.size() / 2;
			int32_t i = step;
			int32_t best_i = -1;
			
			while (true) {
				std::string &current_keyword = index_array[i]["keyword"].as<json::string>();

				if (current_keyword.find(keyword) == 0)
					best_i = i;

				if (current_keyword == keyword || step / 2 == 0)
					break;
				else if (current_keyword < keyword)
					i += (step /= 2);
				else
					i -= (step /= 2);
			}

			// Now we much also check the previous index
			if (i > 0) {
				std::string &prev_keyword = index_array[i - 1]["keyword"].as<json::string>();
				if (prev_keyword.find(keyword) == 0)
					best_i = i - 1;
			}

			if (best_i == -1)
				continue;

			for (json &package : index_array[best_i]["packages"].as<json::array>()) {
				std::string &package_id = package.as<json::string>();
				if (!packages.contains(package_id)) {
					std::string package_url = url + "packages/" + package_id + ".json";
					json package = json::parse(util::download(package_url, volt_path / "cacert.pem"));
					package_url = package["url"].as<json::string>();
					package = json::parse(util::download(package_url, volt_path / "cacert.pem"));
					packages[package_id] = std::make_tuple(
						package["title"].as<json::string>(),
						package["version"].as<json::string>(),
						package["description"].as<json::string>()
					);
				}
			}
		}
	}

	if (packages.size() == 0) {
		std::cout << termcolor::bright_red << "\nNo packages have been found.\n"
				  << termcolor::reset;
		return true;
	}

	std::cout << "\nFound " << packages.size()
			  << (packages.size() == 1 ? " package:\n" : " packages:\n");

	for (auto &package : packages) {
		std::cout << '\n' << std::get<0>(package.second) << " ("
				  << termcolor::bright_green << package.first
				  << termcolor::reset << ") " << std::get<1>(package.second)
				  << '\n' << std::get<2>(package.second) << '\n';
	}

	return true;
}

}

json get_search_index(std::string_view url) {
	fs::path volt_path(std::getenv("VOLT_PATH"));
	fs::path archives_path = volt_path / "archives/";

	std::string index_filename = util::sha256(url) + ".json";
	fs::path index_path = archives_path / index_filename;
	fs::path hash_path = archives_path / (index_filename + ".sha256");

	std::string local_index = fs::is_regular_file(
			index_path) ? util::read_file(index_path) : "";
	std::string local_hash = fs::is_regular_file(
			hash_path) ? util::read_file(hash_path) : "";

	std::string index_url = std::string(url) + "packages.json";
	std::string hash_url = index_url + ".sha256";

	std::string remote_hash = util::download(hash_url, volt_path / "cacert.pem");

	if (local_hash != remote_hash ||
			util::sha256(local_index) != local_hash) {
		std::cout << "Search index is out-dated. Downloading...\n";

		std::string remote_index = util::download(index_url, volt_path / "cacert.pem");
		fs::create_directory(archives_path);

		util::write_file(index_path, local_index = remote_index);
		util::write_file(hash_path, local_hash = remote_hash);
	}

	return json::parse(local_index);
}
