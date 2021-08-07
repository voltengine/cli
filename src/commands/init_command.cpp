#include "commands.hpp"

#include "util/file.hpp"
#include "util/json.hpp"
#include "util/string.hpp"
#include "command_manager.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
using namespace util;

namespace commands {

init_command::init_command() : command(
		"init",
		"",
		"Initializes package.json in current directory.") {}

void init_command::run(const std::vector<std::string> &args) const {
	if (args.size() > 0) {
		std::cout << tc::bright_yellow << "Ignoring extra arguments.\n"
				  << tc::reset;
	}

	fs::path current_path = fs::current_path();
	std::string scope, name, git, description, license;

	std::string default_scope;
	util::shell("git config user.name", [&default_scope](std::string_view data) {
		default_scope += data;
	});
	default_scope.pop_back(); // Pop newline

	std::transform(default_scope.begin(), default_scope.end(),
			default_scope.begin(), ::tolower);

	static const std::regex scope_validator(
			"(?=^.{1,39}$)^[a-z\\d]+(-[a-z\\d]+)*$");
	if (!std::regex_match(default_scope, scope_validator))
		default_scope = "";

	while (true) {
		if (default_scope.empty()) {
			std::cout << tc::bright_green << "Scope: "
					  << tc::reset;
		} else {
			std::cout << tc::bright_green << "Scope ("
					  << tc::reset << default_scope
					  << tc::bright_green << "): "
					  << tc::reset;
		}
		std::getline(std::cin, scope);

		if (scope.empty()) {
			if (!default_scope.empty())
				break;
		} else if (std::regex_match(scope, scope_validator))
			break;

		std::cout << tc::bright_red
				<< "Package scope must be a valid GitHub user/organization name. "
				"You must be organization's administrator to publish packages in its scope.\n"
				<< tc::reset;
	}

	if (scope.empty())
		scope = default_scope;
	
	std::string default_name = current_path.filename().string();
	std::transform(default_name.begin(), default_name.end(),
			default_name.begin(), ::tolower);
	util::replace(default_name, " ", "-");

	static const std::regex name_validator(
			"(?=^.{1,64}$)^[a-z][a-z\\d]*(-[a-z\\d]+)*$");
	if (!std::regex_match(default_name, name_validator))
		default_name = "";

	while (true) {
		if (default_name.empty()) {
			std::cout << tc::bright_green << "Name: "
					  << tc::reset;
		} else {
			std::cout << tc::bright_green << "Name ("
					  << tc::reset << default_name
					  << tc::bright_green << "): "
					  << tc::reset;
		}
		std::getline(std::cin, name);

		if (name.empty()) {
			if (!default_name.empty())
				break;
		} else if (std::regex_match(name, name_validator))
			break;

		std::cout << tc::bright_red
				<< "Package name must comprise only lowercase alphanumeric strings "
				"separated with single hyphens. Name must not start with a digit.\n"
				<< tc::reset;
	}

	if (name.empty())
		name = default_name;

	std::string default_git = "https://github.com/" + scope + '/' + name + ".git";
	while (true) {
		std::cout << tc::bright_green << "Git URL ("
				  << tc::reset << default_git
				  << tc::bright_green << "): "
				  << tc::reset;
		std::getline(std::cin, git);

		if (git.empty()) {
			git = default_git;
			break;
		}

		static const std::regex url_validator(
				"(?=^.{0,2048}$)^(https?|ssh|git|file|ftp):\\/\\/"
				"([^:@\\/]+(:[^:@\\/]+)?@)?[^:@\\/]+(:\\d+)?((?!.*\\/\\/)\\/.*)?$");
		
		if (std::regex_match(git, url_validator))
			break;

		std::cout << tc::bright_red
				<< "Invalid URL.\n"
				<< tc::reset;
	}

	std::cout << tc::bright_green << "Description ("
			  << tc::reset << "42"
			  << tc::bright_green << "): "
			  << tc::reset;
	std::getline(std::cin, description);

	// TODO validate SPDX expression
	std::cout << tc::bright_green << "License ("
			  << tc::reset << "MIT"
			  << tc::bright_green << "): "
			  << tc::reset;
	std::getline(std::cin, license);

	if (description.empty())
		description = "The answer to the Ultimate Question of Life, the Universe, and Everything.";
	if (license.empty())
		license = "MIT";

	util::replace(description, "\\n", "\n");

	json package = json::object();

	package["dependencies"] = json::object();
	package["description"] = description;
	package["git"] = git;
	package["id"] = scope + '/' + name;
	package["keywords"] = json::array();
	package["license"] = license;
	package["version"] = "0.1.0";

	fs::path package_path = current_path / "package.json";
	util::write_file(package_path, util::to_string(package));

	std::cout << tc::bright_green << "\nFile has been written:\n"
			  << tc::reset << package_path.string() << '\n';
}

}
