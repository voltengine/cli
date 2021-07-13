#include "search_command.hpp"

#include "util/crypto.hpp"
#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;
using namespace util;

// [{keyword, [package]}] 
using search_index = std::vector<std::tuple<std::string, std::vector<std::string>>>;

static search_index get_search_index(std::string_view url);

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

	static std::regex re("[a-zA-Z0-9]+");

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

		search_index search_index = get_search_index(url);

		for (std::string keyword : args) {
			std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);

			int32_t step = search_index.size() / 2;
			int32_t i = step;
			
			while (true) {
				std::string &current_keyword = std::get<0>(search_index[i]);

				if (current_keyword == keyword || step / 2 == 0)
					break;
				else if (current_keyword < keyword)
					i += (step /= 2);
				else
					i -= (step /= 2);
			}

			// Now i is in close neighborhood of potential matches
			std::vector<int32_t> matching_indices;

			std::string &current_keyword = std::get<0>(search_index[i]);
			if (current_keyword.substr(0, keyword.size()) ==
					keyword.substr(0, current_keyword.size())) {
				matching_indices.push_back(i);
			}

			// Scan linearly backward, then forward
			int32_t j = i;
			while (true) {
				if (--j < 0)
					break;

				std::string &prev_keyword = std::get<0>(search_index[j]);
				if (prev_keyword.substr(0, keyword.size()) ==
						keyword.substr(0, prev_keyword.size())) {
					matching_indices.push_back(j);
				} else
					break;
			}

			j = i;
			while (true) {
				if (++j >= search_index.size())
					break;

				std::string &next_keyword = std::get<0>(search_index[j]);
				if (next_keyword.substr(0, keyword.size()) ==
						keyword.substr(0, next_keyword.size())) {
					matching_indices.push_back(j);
				} else
					break;
			}

			for (int32_t index : matching_indices) {
				for (std::string &package : std::get<1>(search_index[index])) {
					if (!packages.contains(package)) {
						std::string manifest_url = url + "packages/" + package + ".json";
						json manifest;

						try {
							manifest = json::parse(util::download(manifest_url, cert_path));
						} catch (std::exception &) {
							std::cout << termcolor::bright_yellow
									  << "Manifest unavailable for matching package: " << package << '\n'
					  				  << termcolor::reset;
							continue;
						}

						packages[package] = std::make_tuple(
							manifest["title"].as<json::string>(),
							manifest.as<json::object>().contains("description") ?
									manifest["description"].as<json::string>() : ""
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
				  << termcolor::reset << ")\n";
		
		if (!std::get<1>(package.second).empty())
			std::cout << std::get<1>(package.second) << '\n';
	}

	return;
}

}

search_index get_search_index(std::string_view url) {
	fs::path volt_path = std::getenv("VOLT_PATH");
	fs::path cert_path = volt_path / "cacert.pem";
	fs::path archives_path = volt_path / "archives/";

	std::string index_filename = util::sha256(url) + ".csv";
	fs::path index_path = archives_path / index_filename;
	fs::path hash_path = archives_path / (index_filename + ".sha256");

	std::string local_index = fs::is_regular_file(
			index_path) ? util::read_file(index_path) : "";
	std::string local_hash = fs::is_regular_file(
			hash_path) ? util::read_file(hash_path) : "";

	std::string index_url = std::string(url) + "packages.csv";
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

	search_index index;

	std::stringstream ss(std::move(local_index));
    std::string line;
    while (std::getline(ss, line)) {
        std::vector<std::string> tokens = util::split(line, ",");
		std::swap(tokens.front(), tokens.back());

		std::string keyword = std::move(tokens.back());
		tokens.pop_back();

		index.emplace_back(std::move(keyword), std::move(tokens));
    }

	return index;
}
