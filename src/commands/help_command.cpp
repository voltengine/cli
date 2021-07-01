#include "commands/help_command.hpp"

#include "command_manager.hpp"

namespace commands {

help_command::help_command() : command(
		"help",
		"[{command}]",
		"Prints version info and available commands.\n"
		"Can optionally provide detailed help for a specific command.") {}

void help_command::run(const std::vector<std::string> &args) const {
	if (args.size() == 0) {
		std::cout << termcolor::cyan << "Volt CLI 1.0.0\n\n"
				  << termcolor::reset << "Available commands:\n";

		for (const auto &cmd : command_manager::get_commands()) {
			std::cout << termcolor::red << "volt "
					  << termcolor::reset << cmd.first << "\n";
		}
	} else {
		const auto &cmd = command_manager::find_command(args[0]);

		std::cout << "Usage:\n";
		std::cout << termcolor::red << "volt "
				  << termcolor::reset << cmd->name << " " << cmd->syntax << "\n\n";

		std::cout << "Description:\n";
		std::cout << cmd->description << "\n";
	}
}

}