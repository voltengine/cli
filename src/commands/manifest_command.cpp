#include "manifest_command.hpp"

namespace fs = std::filesystem;

namespace commands {

manifest_command::manifest_command() : command(
		"manifest",
		"(release | sync)",
		"Adds current package version to releases in 'manifest.json' or synchronizes basic information from 'package.json' (description, Git URL, ID, license, publisher, title)."
		"Generates synchronized 'manifest.json' with no releases if file is not present.") {}

void manifest_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0)
		throw std::runtime_error("Required arguments are missing.");

	if (args[0] != "release" && args[0] != "sync")
		throw std::runtime_error("Invalid argument: " + args[0]);

	if (args.size() > 1) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n"
				  << termcolor::reset;
	}

	fs::path package_path = fs::current_directory() / "package.json";
	if (!fs::exists(package_path))
		throw std::runtime_error("");

	fs::path manifest_path = fs::current_directory() / "manifest.json";
	if (!fs::exists(manifest_path)) {
		std::cout << termcolor::bright_yellow << "Manifest does not exist. Generating...\n"
				  << termcolor::reset;
	}

	if (args[0] == "release") {

	} else {

	}
}

}
