#include "commands.hpp"

#include "util/file.hpp"
#include "util/string.hpp"
#include "colors.hpp"
#include "command_manager.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

namespace commands {

init_command::init_command() : command(
		"init",
		"",
		"Initializes \"package.json\" in current directory.") {}

void init_command::run(const std::vector<std::string> &args) const {
	if (args.size() > 0) {
		std::cout << colors::warning << "Ignoring extra arguments.\n\n"
		          << tc::reset;
	}

	fs::path current_path = fs::current_path();
	std::string scope, name, git, description, license;

	std::string default_scope;
	util::shell("git config user.name",
			[&default_scope](std::string_view out) {
		default_scope += out;
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
			std::cout << colors::main << "Scope: "
			          << tc::reset;
		} else {
			std::cout << colors::main << "Scope ("
			          << tc::reset << default_scope
			          << colors::main << "): "
			          << tc::reset;
		}
		std::getline(std::cin, scope);
		if (std::cin.fail())
			std::exit(EXIT_SUCCESS);

		if (scope.empty()) {
			if (!default_scope.empty())
				break;
		} else if (std::regex_match(scope, scope_validator))
			break;

		std::cout << colors::error
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
			std::cout << colors::main << "Name: "
			          << tc::reset;
		} else {
			std::cout << colors::main << "Name ("
			          << tc::reset << default_name
			          << colors::main << "): "
			          << tc::reset;
		}
		std::getline(std::cin, name);
		if (std::cin.fail())
			std::exit(EXIT_SUCCESS);

		if (name.empty()) {
			if (!default_name.empty())
				break;
		} else if (std::regex_match(name, name_validator))
			break;

		std::cout << colors::error
				<< "Package name must comprise only lowercase alphanumeric strings "
				"separated with single hyphens. Name must not start with a digit.\n"
				<< tc::reset;
	}

	if (name.empty())
		name = default_name;

	std::string default_git = "https://github.com/" + scope + '/' + name + ".git";
	while (true) {
		std::cout << colors::main << "Git URL ("
		          << tc::reset << default_git
		          << colors::main << "): "
		          << tc::reset;
		std::getline(std::cin, git);
		if (std::cin.fail())
			std::exit(EXIT_SUCCESS);

		if (git.empty()) {
			git = default_git;
			break;
		}

		static const std::regex url_validator(
				"(?=^.{0,2048}$)^(https?|ssh|git|file|ftp):\\/\\/"
				"([^:@\\/]+(:[^:@\\/]+)?@)?[^:@\\/]+(:\\d+)?((?!.*\\/\\/)\\/.*)?$");
		
		if (std::regex_match(git, url_validator))
			break;

		std::cout << colors::error
				<< "Invalid URL.\n"
				<< tc::reset;
	}

	std::cout << colors::main << "Description ("
	          << tc::reset << "42"
	          << colors::main << "): "
	          << tc::reset;
	std::getline(std::cin, description);
	if (std::cin.fail())
			std::exit(EXIT_SUCCESS);

	// TODO validate SPDX expression
	std::cout << colors::main << "License ("
	          << tc::reset << "MIT"
	          << colors::main << "): "
	          << tc::reset;
	std::getline(std::cin, license);
	if (std::cin.fail())
			std::exit(EXIT_SUCCESS);

	if (description.empty())
		description = "The answer to the Ultimate Question of Life, the Universe, and Everything.";
	if (license.empty())
		license = "MIT";

	util::replace(description, "\\n", "\n");

	nl::json package;

	package["dependencies"] = nl::json::object();
	package["description"] = description;
	package["git"] = git;
	package["id"] = scope + '/' + name;
	package["keywords"] = nl::json::array();
	package["license"] = license;
	package["version"] = "0.1.0";

	// Write package.json
	fs::path package_path = current_path / "package.json";
	util::write_file(package_path, package.dump(1, '\t'));

	std::cout << colors::success << "\nFile was written:\n"
	          << tc::reset << package_path.string() << '\n';

	// Copy the template
	std::cout << "\nCopying template files...";
	fs::path volt_path = common::getenv("VOLT_PATH");
	try {
		fs::copy(volt_path / "template", current_path);
	} catch (...) {
		std::cout << colors::warning << " Failed.\n" << tc::reset;
		return;
	}

	std::cout << colors::success << " Done.\n" << tc::reset;
}

}
