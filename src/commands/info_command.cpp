#include "commands/info_command.hpp"

#include "util/crypto.hpp"
#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;
using namespace util;

static json get_search_index(size_t index);

namespace commands {

info_command::info_command() : command(
		"info",
		"{package-id}",
		"Shows detailed information about a package.") {}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	std::cout << termcolor::bright_green << std::string(ptr, size * nmemb) << termcolor::reset;
	return size * nmemb;
}

bool info_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0) {
		std::cout << termcolor::bright_red << "Package must be provided.\n"
				  << termcolor::reset;
		return false;
	}

	static const std::regex package_name_validator("[0-9a-z-]+");

	std::string package_name = args[0];
	std::string git_url;
	std::string info_url;

	if (!std::regex_match(args[0], package_name_validator)) {
		std::cout << termcolor::bright_red
					<< "Invalid package name.\n"
					<< termcolor::reset;
		return true;
	}

	fs::path volt_path(std::getenv("VOLT_PATH"));
	json archives_json = json::parse(util
			::read_file(volt_path / "archives.json"));
	auto &archives = archives_json.as<json::array>();

	for (auto &archive : archives_json.as<json::array>()) {
		std::string &url = archive.as<json::string>();
		if (url.back() != '/')
			url += '/';

		std::cout << "Checking at '" << url << "'...\n";

		url += "packages/" + args[0] + ".json";
		try {
			json package = json::parse(util::download(url, volt_path / "cacert.pem"));
			git_url = package["git"].as<json::string>();
			info_url = package["info"].as<json::string>();
			break;
		} catch (std::exception &e) {
			std::cout << termcolor::bright_red
						<< "Not found: " << e.what() << '\n'
						<< termcolor::reset;
		}
	}

	if (git_url.empty()) {
		std::cout << termcolor::bright_red
				  << "Package not found in archives.\n"
				  << termcolor::reset;
		return true;
	}

	json info = json::parse(util::download(info_url, volt_path / "cacert.pem"));

	std::cout << '\n' << info["title"].as<json::string>()
			  << " (" << termcolor::bright_green
			  << info["id"].as<json::string>() << termcolor::reset
			  << ") " << info["version"].as<json::string>()
			  << termcolor::bright_green << " @ " << termcolor::reset
			  << info["publisher"].as<json::string>() << '\n';

	std::cout << termcolor::bright_green << "\nGit URL:\n"
			  << termcolor::reset << git_url << '\n';

	if (info.as<json::object>().contains("description")) {
		std::cout << termcolor::bright_green << "\nDescription:\n"
				<< termcolor::reset << info["description"].as<json::string>() << '\n';
	}

	if (info.as<json::object>().contains("dependencies") &&
			!info["dependencies"].as<json::array>().empty()) {
		std::cout << termcolor::bright_green << "\nDependencies:\n"
				<< termcolor::reset;

		for (json &dep : info["dependencies"].as<json::array>()) {
			std::cout << dep.as<json::string>() << '\n';
		}
	}

	if (info.as<json::object>().contains("license")) {
		std::cout << termcolor::bright_green << "\nLicense:\n"
				<< termcolor::reset << info["license"].as<json::string>() << '\n';
	}

	return true;
}

}

// json get_search_index(size_t index, std::string_view url) {
// 	fs::path volt_path(std::getenv("VOLT_PATH"));

// 	std::string index_filename = "archives/" + std::to_string(index) + ".json";
// 	fs::path index_path = volt_path / index_filename;
// 	fs::path hash_path = volt_path / (index_filename + ".sha256");

// 	std::string local_index = util::read_file(index_path);
// 	std::string local_hash = util::read_file(hash_path);

// 	std::string index_url = std::string(url) + "packages.json";
// 	std::string hash_url = index_url + ".sha256";

// 	http::buffer buffer = http::download(hash_url, volt_path / "cacert.pem");
// 	std::string remote_hash(buffer.begin(), buffer.end());

// 	if (local_hash != remote_hash ||
// 			util::sha256(local_index) != local_hash) {
// 		std::cout << "Search index '" << url << "' is out-dated. Fetching...\n";

// 		buffer = http::download(index_url, volt_path / "cacert.pem");
// 		std::string remote_index(buffer.begin(), buffer.end());

// 		util::write_file(index_path, local_index = remote_index);
// 		util::write_file(hash_path, local_hash = remote_hash);
// 	}

// 	return json::parse(local_index);
// }
