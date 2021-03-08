#include <cstdlib>
#include <iostream>

#include "command_manager.hpp"
#include "help_command.hpp"

int main(int argc, char **argv) {
	command_manager::init();

	std::vector<std::string> args;

	if (argc == 1) {
		help_command().run(args);
		return EXIT_SUCCESS;
	}

	std::string name(argv[1]);
	args.resize(argc - 2);
	for (int i = 0; i < argc - 2; i++)
		args[i] = argv[i + 2];
	
	const auto &cmd = command_manager::find_command(name);
	if (cmd)
		cmd->run(args);

	return EXIT_SUCCESS;
}
