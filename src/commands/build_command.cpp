#include "commands.hpp"

#include <nlohmann/json.hpp>

#include "util/file.hpp"
#include "util/string.hpp"
#include "colors.hpp"
#include "common.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;

std::map<std::string, std::string> platforms{
	{ "linux-amd64", "LinuxAMD64" },
	{ "windows-amd64", "WindowsAMD64" },
	{ "windows-x86", "WindowsX86" }
};

namespace commands {

build_command::build_command() : command(
		"build",
		"{platform}",
		"Executes CMake ALL build for release on specified platform at \"./cache/cmake-build/\".\n"
		"Then calls Python scripts in order to bundle assets to \"./build/{platform}/\".\n"
		"Supported platforms:\nlinux-amd64 windows-amd64 windows-x86") {}

void build_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0)
		throw std::runtime_error("Platform not specififed.");
	if (args.size() > 1) {
		std::cout << colors::warning << "Ignoring extra arguments.\n\n"
		          << tc::reset;
	}

	fs::path volt_path = std::getenv("VOLT_PATH");
	auto current_path = fs::current_path();
	auto toolchains_path = volt_path / "cmake" / "toolchains";
	
	// Get path to the CMake toolchain
	if (!platforms.contains(args[0]))
		throw std::runtime_error("Invalid platform.");

	fs::path toolchain_path = toolchains_path / (VOLT_CLI_TOOLCHAIN_PREFIX
			"-" + platforms[args[0]] + ".cmake");

	std::string build_dir = "./cache/cmake-build/";
	if (args.size() != 0)
		fs::remove_all(build_dir);

	common::cmake_build(build_dir, toolchain_path, false, false);

	// Get "./" and "./build/{platform}/"
	auto current_path_str = current_path.string() + '/';
	auto bundle_path = current_path / "build" / args[0];
	auto bundle_path_str = bundle_path.string() + '/';

	#ifdef _WIN32
			util::replace(current_path_str, "\\", "/");
			util::replace(bundle_path_str, "\\", "/");
	#endif

	// Create/Erase the bundle and copy binaries
	fs::remove_all(bundle_path);
	common::copy_cmake_output_binaries(build_dir, bundle_path);

	// Read "./cache/packages.json"
	auto paths_file = current_path / "cache" / "packages.txt";
	auto paths = util::split(util::read_file(paths_file), "\n");

	// Squash package paths into a signle string
	std::string bundle_args;
	for (auto &item : paths) {
		auto path = item.substr(item.find(' ') + 1);
		bundle_args += '"' + path + "\" ";
	}
	bundle_args += '"' + current_path_str + '"';

	// Iterate packages again and execute each one's "bundle.py"
	for (auto &item : paths) {
		size_t delimiter_index = item.find(' ');
		fs::path path = item.substr(delimiter_index + 1);
		auto bundle_script_path = path / "bundle.py";

		if (!fs::exists(bundle_script_path))
			continue;

		std::cout << colors::main << "\nBundling \""
		          << item.substr(0, delimiter_index) << "\":\n" << tc::reset;

		auto bundle_script_path_str = bundle_script_path.string();
#ifdef _WIN32
		util::replace(bundle_script_path_str, "\\", "/");
#endif

		auto cmd = "python \"" + bundle_script_path_str + "\" \"" +
				bundle_path_str + "\" " + bundle_args;
		
		if (util::shell(cmd, [](std::string_view out) {
			std::cout << out;
		}) != 0)
			throw std::runtime_error("Bundling failed.");

		std::cout << "Done.\n";
	}
	
	// So lucky we've got that far o .o Yay! ^^
	std::cout << colors::success << "\nBuild successful at:\n"
	          << tc::reset << bundle_path_str << '\n';
}

}
