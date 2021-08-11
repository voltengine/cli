#include "commands.hpp"

#include "colors.hpp"
#include "command_manager.hpp"

namespace tc = termcolor;

namespace commands {

help_command::help_command() : command(
		"help",
		"[{command}]",
		"Prints version info and available commands.\n"
		"Can optionally provide detailed help for a specific command.") {}

void help_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0) {
		std::cout << colors::main << "Volt CLI 0.1.0\n\n"
				  << tc::reset << "Available commands:\n";

		for (const auto &cmd : command_manager::get_commands()) {
			std::cout << colors::main << "volt "
					  << tc::reset << cmd.second->name
					  << ' ' << cmd.second->syntax << "\n";
		}
	} else {
		if (args.size() > 1) {
			std::cout << colors::warning << "Ignoring extra arguments.\n"
					  << tc::reset;
		}

		const auto &cmd = command_manager::find_command(args[0]);
		if (!cmd)
			throw std::runtime_error("No such command.");

		std::cout << colors::main << "Usage:\nvolt "
				  << tc::reset << cmd->name << " " << cmd->syntax << "\n\n";

		std::cout << colors::main << "Description:\n"
				  << tc::reset << cmd->description << "\n";
	}
}

}
