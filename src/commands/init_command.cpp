#include "init_command.hpp"

#include "util/file.hpp"
#include "util/json.hpp"
#include "util/string.hpp"
#include "command_manager.hpp"

namespace fs = std::filesystem;
using namespace util;

namespace commands {

init_command::init_command() : command(
		"init",
		"",
		"Initializes package.json in current directory.") {}

void init_command::run(const std::vector<std::string> &args) const {
	if (args.size() > 0) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n"
				  << termcolor::reset;
	}

	fs::path current_path = fs::current_path();
	std::string id, title, git, publisher, description, license;

	static std::regex id_validator("[0-9a-z-]+");
	std::string default_id = current_path.filename().string();
	std::transform(default_id.begin(), default_id.end(), default_id.begin(), ::tolower);
	util::replace(default_id, " ", "-");
	if (!std::regex_match(default_id, id_validator))
		default_id = "";

	while (true) {
		if (default_id.empty()) {
			std::cout << termcolor::bright_green << "ID: "
					  << termcolor::reset;
		} else {
			std::cout << termcolor::bright_green << "ID ("
					<< termcolor::reset << default_id
					<< termcolor::bright_green << "): "
					<< termcolor::reset;
		}
		std::getline(std::cin, id);

		if (id.empty()) {
			if (!default_id.empty())
				break;
		} else if (std::regex_match(id, id_validator))
			break;

		std::cout << termcolor::bright_red
				<< "Package ID must comprise only lowercase alphanumerics and hyphens.\n"
				<< termcolor::reset;
	}

	if (id.empty())
		id = default_id;

	auto id_words = util::split(id, "-", true);
	std::string default_title;
	for (std::string &word : id_words)
		default_title += static_cast<char>(std::toupper(word[0])) + word.substr(1) + ' ';
	default_title.pop_back();
	
	std::cout << termcolor::bright_green << "Title ("
			  << termcolor::reset << default_title
			  << termcolor::bright_green << "): "
			  << termcolor::reset;
	std::getline(std::cin, title);

	std::string current_path_str = current_path.string();
#if _WIN32
	util::replace(current_path_str, "\\", "/");
#endif
	std::string default_git = "file:///" + current_path_str + '/';

	while (true) {
		std::cout << termcolor::bright_green << "Git URL ("
				  << termcolor::reset << default_git
				  << termcolor::bright_green << "): "
				  << termcolor::reset;
		std::getline(std::cin, git);

		if (git.empty())
			git = default_git;

		static std::regex validator("(http:\\/|https:\\/|file:\\/\\/)(\\/\\S+)+");
		
		if (std::regex_match(git, validator))
			break;

		std::cout << termcolor::bright_red
				<< "Invalid URL.\n"
				<< termcolor::reset;
	}

	std::cout << termcolor::bright_green << "Publisher ("
			  << termcolor::reset << "Megadodo Publications"
			  << termcolor::bright_green << "): "
			  << termcolor::reset;
	std::getline(std::cin, publisher);

	std::cout << termcolor::bright_green << "Description ("
			  << termcolor::reset << "42"
			  << termcolor::bright_green << "): "
			  << termcolor::reset;
	std::getline(std::cin, description);

	std::cout << termcolor::bright_green << "License ("
			  << termcolor::reset << "MIT"
			  << termcolor::bright_green << "): "
			  << termcolor::reset;
	std::getline(std::cin, license);

	if (title.empty())
		title = default_title;
	if (publisher.empty())
		publisher = "Megadodo Publications";
	if (description.empty())
		description = "The answer to the Ultimate Question of Life, the Universe, and Everything.";
	if (license.empty())
		license = "MIT";

	util::replace(description, "\\n", "\n");

	json package = json::object();

	package["id"] = id;
	package["title"] = title;
	package["git"] = git;
	package["publisher"] = publisher;
	package["description"] = description;
	package["license"] = license;
	package["version"] = "0.1.0";

	fs::path package_path = current_path / "package.json";
	util::write_file(package_path, util::to_string(package));

	std::cout << termcolor::bright_green << "\nFile has been written:\n"
			  << termcolor::reset << package_path.string() << '\n';
}

}
