#include "commands/list_command.hpp"

namespace fs = std::filesystem;

struct package {
	std::string id;
	std::vector<std::string> versions;
};

namespace commands {

list_command::list_command() : command(
		"list",
		"",
		"Displays installed packages.\n") {}

void list_command::run(const std::vector<std::string> &args) const {
#if _WIN32
	fs::path path(fs::path(std::getenv("HOMEDRIVE")) / std::getenv("HOMEPATH"));
#elif __linux__
	fs::path path(std::getenv("HOME"));
#else
	#error "Unsupported OS."
#endif

	path /= ".volt/packages";
	std::vector<package> pkgs;

	if (fs::exists(path) && !fs::is_empty(path)) {
		for (auto &item : fs::directory_iterator(path)) {
			if (!item.is_directory())
				continue;
			
			package pkg;

			auto &path = item.path();
			pkg.id = path.filename().string();

			for (auto &item : fs::directory_iterator(path)) {
				if (!item.is_directory())
					continue;

				auto &path = item.path();
				pkg.versions.push_back(path.filename().string());
			}

			if (pkg.versions.size() > 0)
				pkgs.push_back(pkg);
		}
	}

	if (pkgs.size() == 0) {
		std::cout << termcolor::red << "No packages are installed.\n"
				  << termcolor::reset;
		return;
	}

	std::cout << "Found " << pkgs.size() << " packages:\n\n";

	for (package &pkg : pkgs) {
		std::cout << termcolor::red << pkg.id << '\n'
				  << termcolor::reset;
		
		for (std::string &version : pkg.versions)
			std::cout << '\t' << version << '\n';
	}
}

}
