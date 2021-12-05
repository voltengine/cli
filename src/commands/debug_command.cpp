#include "commands.hpp"

#include "util/system.hpp"
#include "colors.hpp"
#include "command_manager.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;

namespace commands {

debug_command::debug_command() : command(
		"debug",
		"",
		"Builds for development with debugging support and launches the app.") {}

void debug_command::run(const std::vector<std::string> &args) const {
	if (args.size() != 0) {
		std::cout << colors::warning << "Ignoring extra arguments.\n\n"
		          << tc::reset;
	}
	
	common::cmake_build("./cache/cmake/", {}, true, true);

	fs::path current_path = fs::current_path();

	fs::remove_all(current_path / "cache/bin");
	fs::path app_path = common::copy_cmake_output_binaries(
			current_path / "cache/cmake", current_path / "cache");

	std::cout << colors::success << "\nLaunching \""
	          << app_path.string() << "\"...\n"
	          << tc::reset;
	util::start_in_background(app_path);
}

}
