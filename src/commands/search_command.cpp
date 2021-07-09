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

void search_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0) {
		std::cout << termcolor::bright_red << "Keywords must be provided.\n"
				  << termcolor::reset;
		return;
	}

	// ID : title + description
	std::unordered_map<std::string, std::tuple<
			std::string, std::string>> packages;

	fs::path volt_path = std::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";
	json archives_json = json::parse(util
			::read_file(volt_path / "archives.json"));
	auto &archives = archives_json.as<json::array>();

	for (auto &archive : archives_json.as<json::array>()) {
		std::string &url = archive;
		if (url.back() != '/')
			url += '/';

		std::cout << "Searching at '" << url << "'...\n";

		json index = get_search_index(url);
		json::array &index_array = index.as<json::array>();

		for (std::string keyword : args) {
			std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);

			int32_t step = index_array.size() / 2;
			int32_t i = step;
			
			while (true) {
				std::string &current_keyword = index_array[i]["keyword"];

				if (current_keyword == keyword || step / 2 == 0)
					break;
				else if (current_keyword < keyword)
					i += (step /= 2);
				else
					i -= (step /= 2);
			}

			// Now i is in close neighborhood of potential matches
			std::vector<int32_t> matching_indices;

			std::string &current_keyword = index_array[i]["keyword"];
			if (current_keyword.substr(0, keyword.size()) ==
					keyword.substr(0, current_keyword.size())) {
				matching_indices.push_back(i);
			}

			// Scan linearly backward, then forward
			int32_t j = i;
			while (true) {
				if (--j < 0)
					break;

				std::string &prev_keyword = index_array[j]["keyword"];
				if (prev_keyword.substr(0, keyword.size()) ==
						keyword.substr(0, prev_keyword.size())) {
					matching_indices.push_back(j);
				} else
					break;
			}

			j = i;
			while (true) {
				if (++j >= index_array.size())
					break;

				std::string &next_keyword = index_array[j]["keyword"];
				if (next_keyword.substr(0, keyword.size()) ==
						keyword.substr(0, next_keyword.size())) {
					matching_indices.push_back(j);
				} else
					break;
			}

			for (int32_t index : matching_indices) {
				for (json &package : index_array[index]["packages"].as<json::array>()) {
					std::string &package_id = package;
					if (!packages.contains(package_id)) {
						std::string package_url = url + "packages/" + package_id + ".json";
						json package = json::parse(util::download(package_url, cert_path));
						std::string manifest_url = package["manifest"].as<json::string>();
						json manifest;
						try {
							manifest = json::parse(util::download(manifest_url, cert_path));
						} catch (std::exception &) {
							std::cout << termcolor::bright_yellow
									  << "Manifest unavailable for matching package: " << package_id << '\n'
					  				  << termcolor::reset;
							continue;
						}
						packages[package_id] = std::make_tuple(
							manifest["title"].as<json::string>(),
							manifest["description"].as<json::string>()
						);
					}
				}
			}
		}
	}

	if (packages.size() == 0) {
		std::cout << termcolor::bright_red << "\nNo packages have been found.\n"
				  << termcolor::reset;
		return;
	}

	std::cout << "\nFound " << packages.size()
			  << (packages.size() == 1 ? " package:\n" : " packages:\n");

	for (auto &package : packages) {
		std::cout << '\n' << std::get<0>(package.second) << " ("
				  << termcolor::bright_green << package.first
				  << termcolor::reset << ")\n"
				  << std::get<1>(package.second) << '\n';
	}

	return;
}

}

json get_search_index(std::string_view url) {
	fs::path volt_path = std::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";
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

	std::string remote_hash = util::download(hash_url, cert_path);

	if (local_hash != remote_hash ||
			util::sha256(local_index) != local_hash) {
		std::cout << "Search index is out-dated. Downloading...\n";

		std::string remote_index = util::download(index_url, cert_path);
		fs::create_directory(archives_path);

		util::write_file(index_path, local_index = remote_index);
		util::write_file(hash_path, local_hash = remote_hash);
	}

	return json::parse(local_index);
}
