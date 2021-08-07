#include "commands.hpp"

#include "util/file.hpp"
#include "util/json.hpp"

namespace fs = std::filesystem;
namespace tc = termcolor;
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
		std::cout << termcolor::bright_yellow << "Ignoring extra arguments.\n"
				  << termcolor::reset;
	}

	static const std::regex name_validator(
			"(?=^.{1,64}$)^[a-z][a-z\\d]*(-[a-z\\d]+)*$");
	if (!std::regex_match(args[0], name_validator))
		throw std::runtime_error("Invalid package name.");

	fs::path package_path = fs::current_path() / "package.json";

	if (!fs::exists(package_path))
		throw std::runtime_error("No \"package.json\" in this directory.");

	json package = json::parse(read_file(package_path));
	json::object &deps = package["dependencies"];

	if (std::erase_if(deps, [&args](auto &item) {
		return item.first.substr(item.first.find('/') + 1) == args[0];
	}) == 0)
		throw std::runtime_error("Package has no such dependency.");
	
	util::write_file(package_path, util::to_string(package));
	std::cout << tc::bright_green << "\nFile has been written:\n"
			  << tc::reset << package_path.string() << '\n';
}

}
