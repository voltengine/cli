#include "commands.hpp"

#include "util/file.hpp"
#include "colors.hpp"
#include "command_manager.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
namespace nl = nlohmann;
using namespace util;

namespace commands {

uninstall_command::uninstall_command() : command(
		"uninstall",
		"{name}",
		"Removes dependency from \"package.json\".") {}

void uninstall_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0)
		throw std::runtime_error("Dependency name must be provided.");
	
	if (args.size() > 1) {
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n\n"
		          << termcolor::reset;
	}

	static const std::regex name_validator(
			"(?=^.{1,64}$)^[a-z][a-z\\d]*(-[a-z\\d]+)*$");
	if (!std::regex_match(args[0], name_validator))
		throw std::runtime_error("Invalid package name.");

	fs::path package_path = fs::current_path() / "package.json";

	if (!fs::exists(package_path))
		throw std::runtime_error("No \"package.json\" in current directory.");

	nl::json package = nl::json::parse(read_file(package_path));
	nl::json::object_t &deps = package["dependencies"]
			.get_ref<nl::json::object_t &>();

	if (std::erase_if(deps, [&args](auto &item) {
		return item.first.substr(item.first.find('/') + 1) == args[0];
	}) == 0)
		throw std::runtime_error("Package has no such dependency.");
	
	util::write_file(package_path, package.dump(1, '\t'));
	std::cout << colors::success << "\nFile was written:\n"
	          << tc::reset << package_path.string() << "\n\n";

	command_manager::find_command("install")
			->run(std::vector<std::string>());
}

}
