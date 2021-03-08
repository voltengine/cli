#include "help_command.hpp"

#include <iostream>

#include "command_manager.hpp"

help_command::help_command() : command(
		"help",
		"[{command}]",
		"Prints version info and a list of commands.\n"
		"Can optionally provide detailed help for a specific command.") {}

void help_command::run(const std::vector<std::string> &args) const {
	std::cout << "Volt CLI version 1.0.0\n\n";

	if (args.size() == 0) {
		std::cout << "Available commands:\n";

		const auto &cmds = command_manager::get_commands();
		for (const auto &cmd : cmds) {
			std::cout << "volt " << cmd->name << "\n";
		}
	} else {
		const auto &cmd = command_manager::find_command(args[0]);

		std::cout << "Usage:\n";
		std::cout << "volt " << cmd->name << " " << cmd->syntax << "\n\n";

		std::cout << "Description:\n";
		std::cout << cmd->description << "\n";
	}

	std::cout << std::flush;
}
